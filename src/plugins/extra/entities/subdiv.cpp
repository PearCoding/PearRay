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

#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/osd/cpuEvaluator.h>

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
		PR_LOG(L_INFO) << "Caching mesh " << name() << " [" << boost::filesystem::path(mCNTFile) << "]";

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

class SubdivMeshEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<MeshContainer> refineMesh(const std::shared_ptr<MeshContainer>& originalMesh, int maxLevel)
	{
		// TODO: Better use stencil method, as we do not need levels inbetween

		auto vertsPerFace   = originalMesh->faceVertexCounts();
		const auto& indices = originalMesh->indices();

		OpenSubdiv::Far::TopologyDescriptor desc;
		desc.numFaces			= originalMesh->faceCount();
		desc.numVertices		= originalMesh->nodeCount();
		desc.numVertsPerFace	= (const int*)vertsPerFace.data();
		desc.vertIndicesPerFace = (const int*)indices.data();

		auto type = OpenSubdiv::Sdc::SCHEME_CATMARK;
		OpenSubdiv::Sdc::Options options;
		options.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

		OpenSubdiv::Far::TopologyRefiner* refiner = OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor>::Create(
			desc,
			OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor>::Options(type, options));

		refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(maxLevel));

		PR_LOG(L_INFO) << "From " << desc.numVertices << " to " << refiner->GetNumVerticesTotal() << std::endl;

		// TODO
		return originalMesh;
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