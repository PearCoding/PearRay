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

	int32 mLightID;
	std::vector<uint32> mMaterials;
	TriMesh mMesh;
};

using namespace OpenSubdiv;
class SubdivMeshEntityFactory : public IEntityFactory {
public:
	Far::TopologyDescriptor::FVarChannel fvarChannels[1];
	std::vector<uint32> fvarIndices;

	Far::TopologyRefiner* setupRefiner(const std::shared_ptr<MeshContainer>& originalMesh, int maxLevel, bool adaptive)
	{
		PR_PROFILE_THIS;

		auto vertsPerFace   = originalMesh->faceVertexCounts();
		const auto& indices = originalMesh->indices();

		Far::TopologyDescriptor desc;
		desc.numFaces			= originalMesh->faceCount();
		desc.numVertices		= originalMesh->nodeCount();
		desc.numVertsPerFace	= (const int*)vertsPerFace.data();
		desc.vertIndicesPerFace = (const int*)indices.data();

		if (originalMesh->features() & MF_HAS_MATERIAL) {

			fvarChannels[0].numValues = originalMesh->faceCount();

			fvarIndices.clear();
			fvarIndices.reserve(originalMesh->indices().size());
			for (size_t f = 0; f < originalMesh->faceCount(); ++f) {
				size_t elems = originalMesh->faceVertexCount(f);
				for (size_t i = 0; i < elems; ++i)
					fvarIndices.push_back(f);
			}

			fvarChannels[0].valueIndices = (const int*)fvarIndices.data();

			desc.numFVarChannels = 1;
			desc.fvarChannels	= fvarChannels;
		}

		auto type = Sdc::SCHEME_CATMARK;
		Sdc::Options sdc_options;
		sdc_options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);
		sdc_options.SetFVarLinearInterpolation(Sdc::Options::FVAR_LINEAR_NONE);

		Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(
			desc,
			Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(type, sdc_options));

		if (!adaptive) {
			Far::TopologyRefiner::UniformOptions ref_opts(maxLevel);
			ref_opts.fullTopologyInLastLevel = true;
			refiner->RefineUniform(ref_opts);
		} else {
			Far::TopologyRefiner::AdaptiveOptions ref_opts(maxLevel);
			ref_opts.orderVerticesFromFacesFirst = true;
			refiner->RefineAdaptive(ref_opts);
		}

		return refiner;
	}

	Far::StencilTable const* setupStencilTable(Far::TopologyRefiner* refiner, int interp)
	{
		Far::StencilTableFactory::Options stencil_options;
		stencil_options.generateIntermediateLevels = false;
		stencil_options.generateOffsets			   = true;
		stencil_options.interpolationMode		   = interp;

		return Far::StencilTableFactory::Create(*refiner, stencil_options);
	}

	Osd::CpuVertexBuffer* refineData(const float* source, size_t elems, size_t vertices, Far::StencilTable const* stencilTable)
	{
		Osd::BufferDescriptor srcDesc(0, elems, elems);
		Osd::CpuVertexBuffer* srcbuffer = Osd::CpuVertexBuffer::Create(elems, vertices);
		Osd::BufferDescriptor dstDesc(0, elems, elems);
		Osd::CpuVertexBuffer* dstbuffer = Osd::CpuVertexBuffer::Create(elems, stencilTable->GetNumStencils());

		srcbuffer->UpdateData(source, 0, vertices);

#if defined(OPENSUBDIV_HAS_OPENMP)
		Osd::OmpEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
#elif defined(OPENSUBDIV_HAS_TBB)
		Osd::TbbEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
#else
		Osd::CpuEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
#endif

		delete srcbuffer;
		return dstbuffer;
	}

	void refineVertexData(const std::shared_ptr<MeshContainer>& originalMesh,
						  std::shared_ptr<MeshContainer>& refineMesh,
						  Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->vertices().data(), 3,
												  originalMesh->nodeCount(), stencilTable);

		std::vector<float> dstVerts(3 * stencilTable->GetNumStencils());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setVertices(dstVerts);

		delete buffer;
	}

	void refineUVData(const std::shared_ptr<MeshContainer>& originalMesh,
					  std::shared_ptr<MeshContainer>& refineMesh,
					  Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->uvs().data(), 2,
												  originalMesh->nodeCount(), stencilTable);

		std::vector<float> dstVerts(2 * stencilTable->GetNumStencils());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setUVs(dstVerts);

		delete buffer;
	}

	// TODO: Check
	void refineVelocityData(const std::shared_ptr<MeshContainer>& originalMesh,
							std::shared_ptr<MeshContainer>& refineMesh,
							Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->velocities().data(), 3,
												  originalMesh->nodeCount(), stencilTable);

		std::vector<float> dstVerts(3 * stencilTable->GetNumStencils());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setVelocities(dstVerts);

		delete buffer;
	}

	// Approach without all convert buffers possible?
	void refineMaterialData(const std::shared_ptr<MeshContainer>& originalMesh,
							std::shared_ptr<MeshContainer>& refineMesh,
							Far::StencilTable const* stencilTable,
							Far::TopologyRefiner* refiner)
	{
		PR_PROFILE_THIS;
		Far::TopologyLevel const& refLevel = refiner->GetLevel(refiner->GetMaxLevel());

		std::vector<float> slotsAsFloat(originalMesh->faceCount());
		uint32 max = 0;
		for (size_t i = 0; i < slotsAsFloat.size(); ++i) {
			uint32 v		= originalMesh->materialSlot(i);
			max				= std::max(max, v);
			slotsAsFloat[i] = v;
		}

		Osd::CpuVertexBuffer* buffer = refineData(slotsAsFloat.data(), 1,
												  originalMesh->nodeCount(), stencilTable);

		std::vector<float> dstVerts(refLevel.GetNumFVarValues());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());

		std::vector<uint32> slots(refineMesh->faceCount());
		for (size_t i = 0; i < slots.size(); ++i) {
			Far::ConstIndexArray fverts = refLevel.GetFaceFVarValues(i);
			PR_ASSERT(fverts.size() > 0, "Invalid return by GetFaceFVarValues");
			slots[i] = std::min(max, std::max(0u, static_cast<uint32>(std::floor(dstVerts[fverts[0]]))));
		}
		refineMesh->setMaterialSlots(slots);

		delete buffer;
	}

	std::shared_ptr<MeshContainer> refineMesh(const std::shared_ptr<MeshContainer>& originalMesh, int maxLevel, bool adaptive)
	{
		PR_PROFILE_THIS;

		if (maxLevel < 1)
			return originalMesh;

		Far::TopologyRefiner* refiner			  = setupRefiner(originalMesh, maxLevel, adaptive);
		Far::StencilTable const* stencilTable	 = setupStencilTable(refiner, Far::StencilTableFactory::INTERPOLATE_VERTEX);
		Far::StencilTable const* fvarStencilTable = (originalMesh->features() & MF_HAS_MATERIAL)
														? setupStencilTable(refiner, Far::StencilTableFactory::INTERPOLATE_FACE_VARYING)
														: nullptr;

		Far::TopologyLevel const& refLevel = refiner->GetLevel(maxLevel);
		std::vector<uint32> new_indices(refLevel.GetNumFaceVertices());
		std::vector<uint8> new_vertsPerFace(refLevel.GetNumFaces());

		PR_ASSERT(refLevel.GetNumVertices() == stencilTable->GetNumStencils(), "Expected to be same!");

		PR_LOG(L_INFO) << "Subdivision(L=" << maxLevel << ") [V=" << originalMesh->nodeCount() << " F=" << originalMesh->faceCount() << " I=" << originalMesh->indices().size()
					   << "] -> [V=" << refLevel.GetNumVertices() << " F=" << new_vertsPerFace.size() << " I=" << new_indices.size() << "]" << std::endl;
		size_t indC = 0;
		for (int face = 0; face < refLevel.GetNumFaces(); ++face) {
			Far::ConstIndexArray fverts = refLevel.GetFaceVertices(face);
			new_vertsPerFace[face]		= fverts.size();

			for (int i = 0; i < fverts.size(); ++i)
				new_indices[indC++] = fverts[i];
		}
		PR_ASSERT(indC == new_indices.size(), "Invalid subdivision calculation");

		// Setup mesh
		std::shared_ptr<MeshContainer> refineMesh = std::make_shared<MeshContainer>();

		refineMesh->setIndices(new_indices);
		refineMesh->setFaceVertexCount(new_vertsPerFace);
		refineVertexData(originalMesh, refineMesh, stencilTable);
		refineMesh->buildNormals();

		if (originalMesh->features() & MF_HAS_UV)
			refineUVData(originalMesh, refineMesh, stencilTable);

		if (originalMesh->features() & MF_HAS_MATERIAL)
			refineMaterialData(originalMesh, refineMesh, fvarStencilTable, refiner);

		if (originalMesh->features() & MF_HAS_VELOCITY)
			refineVelocityData(originalMesh, refineMesh, stencilTable);

		// Remove buffers
		std::vector<uint32> null;
		fvarIndices.swap(null);

		delete refiner;
		delete stencilTable;
		if (fvarStencilTable)
			delete fvarStencilTable;

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
		bool adaptive	  = reg.getForObject<bool>(RG_ENTITY, uuid, "adaptive", false);

		std::shared_ptr<MeshContainer> originalMesh = env.getMesh(mesh_name);
		if (!originalMesh)
			return nullptr;

		if (!originalMesh->isValid()) {
			PR_LOG(L_ERROR) << "Mesh " << mesh_name << " is invalid." << std::endl;
			return nullptr;
		}

		std::shared_ptr<MeshContainer> refinedMesh = refineMesh(originalMesh, maxSubdivision, adaptive);
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