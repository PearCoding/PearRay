#include "CacheManager.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "mesh/TriMesh.h"

#include <boost/filesystem.hpp>

#include <opensubdiv/far/stencilTableFactory.h>
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/osd/cpuEvaluator.h>
#include <opensubdiv/osd/cpuVertexBuffer.h>

#ifdef OPENSUBDIV_HAS_OPENMP
#include <opensubdiv/osd/ompEvaluator.h>
#endif

#ifdef OPENSUBDIV_HAS_TBB
#include <opensubdiv/osd/tbbEvaluator.h>
#endif

namespace PR {

class SubdivMeshEntity : public IEntity {
public:
	ENTITY_CLASS

	SubdivMeshEntity(uint32 id, const std::string& name,
					 const std::wstring& cnt_file,
					 const std::shared_ptr<MeshContainer>& mesh,
					 const std::vector<uint32>& materials,
					 int32 lightID)
		: IEntity(id, name)
		, mCNTFile(cnt_file)
		, mLightID(lightID)
		, mMaterials(materials)
		, mMesh(mesh)
	{
	}
	virtual ~SubdivMeshEntity() {}

	std::string type() const override
	{
		return "subdivmesh";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		return mMesh.surfaceArea(id);
	}

	bool isCollidable() const override
	{
		return mMesh.isCollidable();
	}

	float collisionCost() const override
	{
		return mMesh.collisionCost();
	}

	BoundingBox localBoundingBox() const override
	{
		return mMesh.localBoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh.checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			insert(i, out.MaterialID,
				   extract(i, out.MaterialID) < mMaterials.size()
					   ? mMaterials.at(extract(i, out.MaterialID))
					   : 0);
		}
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh.checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.MaterialID  = out.MaterialID < mMaterials.size()
							 ? mMaterials.at(out.MaterialID)
							 : 0;
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const override
	{
		PR_PROFILE_THIS;
		return mMesh.pickRandomPoint(rnd, faceID, pdf);
	}

	void provideGeometryPoint(uint32 faceID, float u, float v,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		mMesh.provideGeometryPoint(faceID, u, v, pt);

		// Global
		pt.P  = transform() * pt.P;
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = pt.MaterialID < mMaterials.size()
							? mMaterials.at(pt.MaterialID)
							: 0;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	void beforeSceneBuild() override
	{
		PR_LOG(L_INFO) << "Caching mesh " << name() << " [" << boost::filesystem::path(mCNTFile) << "]" << std::endl;

		mMesh.build(mCNTFile);
		IEntity::beforeSceneBuild();
	}

private:
	std::wstring mCNTFile;
	bool mLoadOnly;

	int32 mLightID;
	std::vector<uint32> mMaterials;
	TriMesh mMesh;
};

template <typename T, int N>
class DataWalker {
public:
	std::array<std::vector<T>, N> Array;
	inline DataWalker(const std::array<std::vector<T>, N>& array)
		: Array(array)
	{
	}

	struct Vertex {
		size_t Index;
		DataWalker* Walker;
		inline Vertex(size_t index, DataWalker* walker)
			: Index(index)
			, Walker(walker)
		{
			PR_ASSERT(walker, "Expected valid walker!");
		}

		Vertex(const Vertex& other) = default;

		inline Vertex& operator=(const Vertex& other)
		{
			for (int i = 0; i < N; ++i)
				Walker->Array[i][Index] = other.Walker->Array[i][other.Index];
			return *this;
		}

		inline void Clear(void* = nullptr)
		{
			for (int i = 0; i < N; ++i)
				Walker->Array[i][Index] = T(0);
		}

		inline void AddWithWeight(Vertex const& src, float weight)
		{
			for (int i = 0; i < N; ++i)
				Walker->Array[i][Index] += weight * src.Walker->Array[i][src.Index];
		}
	};

	Vertex operator[](size_t index)
	{
		return Vertex(index, this);
	}

	Vertex operator[](size_t index) const
	{
		return Vertex(index, const_cast<DataWalker*>(this));
	}
};

using namespace OpenSubdiv;
class SubdivMeshEntityFactory : public IEntityFactory {
public:
	Far::TopologyRefiner* setupRefiner(const std::shared_ptr<MeshContainer>& originalMesh, int maxLevel)
	{
		auto vertsPerFace   = originalMesh->faceVertexCounts();
		const auto& indices = originalMesh->indices();

		Far::TopologyDescriptor desc;
		desc.numFaces			= originalMesh->faceCount();
		desc.numVertices		= originalMesh->nodeCount();
		desc.numVertsPerFace	= (const int*)vertsPerFace.data();
		desc.vertIndicesPerFace = (const int*)indices.data();

		auto type = Sdc::SCHEME_CATMARK;
		Sdc::Options sdc_options;
		sdc_options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

		Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(
			desc,
			Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(type, sdc_options));

		// Uniform
		refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(maxLevel));

		return refiner;
	}

	Far::StencilTable const* setupStencilTable(Far::TopologyRefiner* refiner)
	{
		Far::StencilTableFactory::Options stencil_options;
		stencil_options.generateIntermediateLevels = false;
		stencil_options.generateOffsets			   = true;

		return OpenSubdiv::Far::StencilTableFactory::Create(*refiner, stencil_options);
	}

	std::shared_ptr<MeshContainer> refineMesh(const std::shared_ptr<MeshContainer>& originalMesh, int maxLevel)
	{
		if (maxLevel < 1)
			return originalMesh;

		Far::TopologyRefiner* refiner		  = setupRefiner(originalMesh, maxLevel);
		Far::StencilTable const* stencilTable = setupStencilTable(refiner);

		int nstencils = stencilTable->GetNumStencils();

		// Refine vertex data
		std::vector<float> v[3]  = { originalMesh->vertices(0), originalMesh->vertices(1), originalMesh->vertices(2) };
		std::vector<float> n[3]  = { originalMesh->normals(0), originalMesh->normals(1), originalMesh->normals(2) };
		std::vector<float> uv[2] = { originalMesh->uvs(0), originalMesh->uvs(1) };

		for (int i = 0; i < 3; ++i) {
			v[i].resize(nstencils);
			n[i].resize(nstencils);
			if (originalMesh->features() & MF_HAS_MATERIAL && i < 2)
				uv[i].resize(nstencils);
		}

		DataWalker<float, 6> src({ originalMesh->vertices(0), originalMesh->vertices(1), originalMesh->vertices(2),
								   originalMesh->normals(0), originalMesh->normals(1), originalMesh->normals(2) });
		DataWalker<float, 6> dst({ v[0], v[1], v[2],
								   n[0], n[1], n[2] });
		stencilTable->UpdateValues(src, dst);

		OpenSubdiv::Far::TopologyLevel const& refLevel = refiner->GetLevel(maxLevel);
		std::vector<uint32> new_indices(refLevel.GetNumFaceVertices());
		std::vector<uint8> new_vertsPerFace(refLevel.GetNumFaces());

		PR_ASSERT(refLevel.GetNumVertices() == nstencils, "Expected to be same!");

		PR_LOG(L_INFO) << "Subdivision(L=" << maxLevel << ") from V=" << originalMesh->nodeCount() << " F=" << originalMesh->faceCount() << " I=" << originalMesh->indices().size()
					   << " to V=" << nstencils << " F=" << new_vertsPerFace.size() << " I=" << new_indices.size() << std::endl;
		size_t indC = 0;
		for (int face = 0; face < refLevel.GetNumFaces(); ++face) {
			OpenSubdiv::Far::ConstIndexArray fverts = refLevel.GetFaceVertices(face);
			new_vertsPerFace[face]					= fverts.size();

			for (int i = 0; i < fverts.size(); ++i)
				new_indices[indC++] = fverts[i];
		}

		delete refiner;
		PR_ASSERT(indC == new_indices.size(), "Invalid subdivision calculation");

		// Setup mesh
		std::shared_ptr<MeshContainer> refineMesh = std::make_shared<MeshContainer>();
		refineMesh->setVertices(v[0], v[1], v[2]);
		refineMesh->setNormals(n[0], n[1], n[2]);
		refineMesh->setIndices(new_indices);
		refineMesh->setFaceVertexCount(new_vertsPerFace);

		return refineMesh;
	}

	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg   = env.registry();
		std::string name	  = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		std::string mesh_name = reg.getForObject<std::string>(RG_ENTITY, uuid, "mesh", "");

		std::vector<std::string> matNames = reg.getForObject<std::vector<std::string>>(
			RG_ENTITY, uuid, "materials", std::vector<std::string>());

		std::vector<uint32> materials;
		for (std::string n : matNames) {
			std::shared_ptr<IMaterial> mat = env.getMaterial(n);
			if (mat)
				materials.push_back(mat->id());
		}

		std::string emsName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "emission", "");
		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = ems->id();

		int maxSubdivision = reg.getForObject<int>(RG_ENTITY, uuid, "max_subdivision", 4);

		std::shared_ptr<MeshContainer> originalMesh = env.getMesh(mesh_name);
		if (!originalMesh)
			return nullptr;

		if (!originalMesh->isValid()) {
			PR_LOG(L_ERROR) << "Mesh " << mesh_name << " is invalid." << std::endl;
			return nullptr;
		}

		std::shared_ptr<MeshContainer> refinedMesh = refineMesh(originalMesh, maxSubdivision);
		return std::make_shared<SubdivMeshEntity>(id, name,
												  env.cacheManager()->requestFile("mesh", name + ".cnt"),
												  refinedMesh,
												  materials, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "subdiv", "subdivision", "opensubdiv" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SubdivMeshEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)