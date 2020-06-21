#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "cache/Cache.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "mesh/MeshFactory.h"

#include <filesystem>
#include <cctype>

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
					 const std::shared_ptr<Mesh>& mesh,
					 const std::vector<uint32>& materials,
					 int32 lightID)
		: IEntity(id, name)
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
		return mMesh->surfaceArea(id);
	}

	bool isCollidable() const override
	{
		return mMesh->isCollidable();
	}

	float collisionCost() const override
	{
		return mMesh->collisionCost();
	}

	BoundingBox localBoundingBox() const override
	{
		return mMesh->localBoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.transformDistance(out.HitDistance,
													   transform().linear());
		out.EntityID	= simdpp::make_uint(id());

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			insert(i, out.MaterialID,
				   extract(i, out.MaterialID) < mMaterials.size()
					   ? mMaterials.at(extract(i, out.MaterialID))
					   : 0);
		}
	}

	void checkCollision(const Ray& in, HitPoint& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.transformDistance(out.HitDistance,
													   transform().linear());
		out.EntityID	= id();
		out.MaterialID	= out.MaterialID < mMaterials.size()
							 ? mMaterials.at(out.MaterialID)
							 : 0;
	}

	Vector3f pickRandomParameterPoint(const Vector3f& /*view*/, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		PR_PROFILE_THIS;
		return mMesh->pickRandomParameterPoint(rnd, faceID, pdf);
	}

	void provideGeometryPoint(const Vector3f&, uint32 faceID, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		mMesh->provideGeometryPoint(faceID, parameter, pt);

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

private:
	int32 mLightID;
	std::vector<uint32> mMaterials;
	std::shared_ptr<Mesh> mMesh;
};

enum OpenSubdivComputationMode {
	OSCM_CPU = 0,
	OSCM_OPENMP,
	OSCM_TBB
};

using namespace OpenSubdiv;
struct SubdivisionOptions {
	int MaxLevel							  = 4;
	bool Adaptive							  = false;
	int Scheme								  = Sdc::SCHEME_CATMARK;
	int BoundaryInterpolation				  = Sdc::Options::VTX_BOUNDARY_EDGE_ONLY;
	int FVarInterpolation					  = Sdc::Options::FVAR_LINEAR_ALL;
	int UVInterpolation						  = Far::StencilTableFactory::INTERPOLATE_VARYING;
	bool LimitNormals						  = true;
	OpenSubdivComputationMode ComputationMode = OSCM_OPENMP;
};

class SubdivMeshEntityPlugin : public IEntityPlugin {
public:
	Far::TopologyDescriptor::FVarChannel fvarChannels[1];
	std::vector<uint32> fvarIndices;

	Far::TopologyRefiner* setupRefiner(const MeshBase* originalMesh, const SubdivisionOptions& opts)
	{
		PR_PROFILE_THIS;

		auto vertsPerFace	= originalMesh->faceVertexCounts();
		const auto& indices = originalMesh->indices();

		Far::TopologyDescriptor desc;
		desc.numFaces			= static_cast<int>(originalMesh->faceCount());
		desc.numVertices		= static_cast<int>(originalMesh->nodeCount());
		desc.numVertsPerFace	= (const int*)vertsPerFace.data();
		desc.vertIndicesPerFace = (const Far::Index*)indices.data();

		static_assert(sizeof(uint32) == sizeof(int) && sizeof(uint32) == sizeof(Far::Index),
					  "Above code is depending on it. (Maybe fix it!)");

		if (originalMesh->features() & MF_HAS_MATERIAL) {
			fvarIndices.clear();
			fvarIndices.reserve(originalMesh->indices().size());
			for (size_t f = 0; f < originalMesh->faceCount(); ++f) {
				size_t elems = originalMesh->faceVertexCount(f);
				for (size_t i = 0; i < elems; ++i)
					fvarIndices.push_back(static_cast<uint32>(f));
			}

			PR_ASSERT(originalMesh->indices().size() == fvarIndices.size(), "Invalid index calculation");

			fvarChannels[0].numValues	 = static_cast<int>(fvarIndices.size());
			fvarChannels[0].valueIndices = (const Far::Index*)fvarIndices.data();

			desc.numFVarChannels = 1;
			desc.fvarChannels	 = fvarChannels;
		}

		Sdc::Options sdc_options;
		sdc_options.SetVtxBoundaryInterpolation((Sdc::Options::VtxBoundaryInterpolation)opts.BoundaryInterpolation);
		sdc_options.SetFVarLinearInterpolation((Sdc::Options::FVarLinearInterpolation)opts.FVarInterpolation);

		Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(
			desc,
			Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options((Sdc::SchemeType)opts.Scheme, sdc_options));

		if (!opts.Adaptive) {
			Far::TopologyRefiner::UniformOptions ref_opts(opts.MaxLevel);
			ref_opts.fullTopologyInLastLevel = true;
			refiner->RefineUniform(ref_opts);
		} else {
			Far::TopologyRefiner::AdaptiveOptions ref_opts(opts.MaxLevel);
			//ref_opts.orderVerticesFromFacesFirst = true;
			ref_opts.useSingleCreasePatch = false;
			ref_opts.considerFVarChannels = (originalMesh->features() & MF_HAS_MATERIAL);
			refiner->RefineAdaptive(ref_opts);
		}

		return refiner;
	}

	// TODO
	/*Far::LimitStencilTable const* setupLimitStencilTable(Far::TopologyRefiner* refiner, int interp)
	{
		Far::LimitStencilTableFactory::Options stencil_options;
		stencil_options.interpolationMode	  = interp;
		stencil_options.generate1stDerivatives = true;
		stencil_options.generate2ndDerivatives = false;

		//return Far::LimitStencilTableFactory::Create(*refiner, stencil_options);
		return nullptr;
	}*/

	Far::StencilTable const* setupStencilTable(Far::TopologyRefiner* refiner, int interp)
	{
		Far::StencilTableFactory::Options stencil_options;
		stencil_options.generateIntermediateLevels = false;
		stencil_options.generateOffsets			   = true;
		stencil_options.interpolationMode		   = interp;

		return Far::StencilTableFactory::Create(*refiner, stencil_options);
	}

	Osd::CpuVertexBuffer* refineData(const float* source, int elems,
									 int coarseElems, int refElems,
									 OpenSubdivComputationMode mode,
									 Far::StencilTable const* stencilTable,
									 Osd::CpuVertexBuffer** limit = nullptr)
	{

		Osd::BufferDescriptor srcDesc(0, elems, elems);
		Osd::CpuVertexBuffer* srcbuffer = Osd::CpuVertexBuffer::Create(elems, coarseElems);
		srcbuffer->UpdateData(source, 0, coarseElems);

		Osd::BufferDescriptor dstDesc(0, elems, elems);
		Osd::CpuVertexBuffer* dstbuffer = Osd::CpuVertexBuffer::Create(elems, refElems);

		if (limit) {
			limit[0] = Osd::CpuVertexBuffer::Create(elems, refElems);
			limit[1] = Osd::CpuVertexBuffer::Create(elems, refElems);
			switch (mode) {
			default:
			case OSCM_CPU:
				Osd::CpuEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, limit[0], dstDesc, limit[1], dstDesc, (Far::LimitStencilTable const*)stencilTable);
				break;
#if defined(OPENSUBDIV_HAS_OPENMP)
			case OSCM_OPENMP:
				Osd::OmpEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, limit[0], dstDesc, limit[1], dstDesc, (Far::LimitStencilTable const*)stencilTable);
				break;
#endif
#if defined(OPENSUBDIV_HAS_TBB)
			case OSCM_TBB:
				Osd::TbbEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, limit[0], dstDesc, limit[1], dstDesc, (Far::LimitStencilTable const*)stencilTable);
				break;
#endif
			}
		} else {
			switch (mode) {
			default:
			case OSCM_CPU:
				Osd::CpuEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
				break;
#if defined(OPENSUBDIV_HAS_OPENMP)
			case OSCM_OPENMP:
				Osd::OmpEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
				break;
#endif
#if defined(OPENSUBDIV_HAS_TBB)
			case OSCM_TBB:
				Osd::TbbEvaluator::EvalStencils(srcbuffer, srcDesc, dstbuffer, dstDesc, stencilTable);
				break;
#endif
			}
		}

		delete srcbuffer;
		return dstbuffer;
	}

	void refineVertexData(const MeshBase* originalMesh,
						  std::unique_ptr<MeshBase>& refineMesh,
						  OpenSubdivComputationMode mode,
						  Far::TopologyRefiner* refiner,
						  Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		int coarseElems				 = refiner->GetLevel(0).GetNumVertices();
		int refElems				 = refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices();
		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->vertices().data(), 3, coarseElems, refElems, mode, stencilTable);

		std::vector<float> dstVerts(3 * refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setVertices(dstVerts);

		delete buffer;
	}

	void refineUVData(const MeshBase* originalMesh,
					  std::unique_ptr<MeshBase>& refineMesh,
					  OpenSubdivComputationMode mode,
					  Far::TopologyRefiner* refiner,
					  Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		int coarseElems				 = refiner->GetLevel(0).GetNumVertices();
		int refElems				 = refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices();
		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->uvs().data(), 2, coarseElems, refElems, mode, stencilTable);

		std::vector<float> dstVerts(2 * refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setUVs(dstVerts);

		delete buffer;
	}

	// TODO: Check
	void refineVelocityData(const MeshBase* originalMesh,
							std::unique_ptr<MeshBase>& refineMesh,
							OpenSubdivComputationMode mode,
							Far::TopologyRefiner* refiner,
							Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;

		int coarseElems				 = refiner->GetLevel(0).GetNumVertices();
		int refElems				 = refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices();
		Osd::CpuVertexBuffer* buffer = refineData(originalMesh->velocities().data(), 3, coarseElems, refElems, mode, stencilTable);

		std::vector<float> dstVerts(3 * refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices());
		memcpy(dstVerts.data(), buffer->BindCpuBuffer(), sizeof(float) * dstVerts.size());
		refineMesh->setVelocities(dstVerts);

		delete buffer;
	}

	// Approach without all convert buffers possible?
	void refineMaterialData(const MeshBase* originalMesh,
							std::unique_ptr<MeshBase>& refineMesh,
							OpenSubdivComputationMode mode,
							Far::TopologyRefiner* refiner,
							Far::StencilTable const* stencilTable)
	{
		PR_PROFILE_THIS;
		Far::TopologyLevel const& refLevel = refiner->GetLevel(refiner->GetMaxLevel());

		//int coarseElems = refiner->GetLevel(0).GetNumFVarValues();
		int refElems = refLevel.GetNumFVarValues();

		std::vector<float> slotsAsFloat(originalMesh->faceCount());
		uint32 max = 0;
		for (size_t i = 0; i < slotsAsFloat.size(); ++i) {
			uint32 v		= originalMesh->materialSlot(i);
			max				= std::max(max, v);
			slotsAsFloat[i] = static_cast<float>(v);
		}

		Osd::CpuVertexBuffer* buffer = refineData(slotsAsFloat.data(), 1, (int)slotsAsFloat.size(), refElems, mode, stencilTable);

		const float* dstVerts = buffer->BindCpuBuffer();
		std::vector<uint32> slots(refineMesh->faceCount());
		for (size_t i = 0; i < slots.size(); ++i) {
			Far::ConstIndexArray fverts = refLevel.GetFaceFVarValues(static_cast<Far::Index>(i));
			PR_ASSERT(fverts.size() > 0, "Invalid return by GetFaceFVarValues");
			slots[i] = std::min(max, std::max(0u, static_cast<uint32>(std::floor(dstVerts[fverts[0]]))));
		}
		refineMesh->setMaterialSlots(slots);

		delete buffer;
	}

	std::unique_ptr<MeshBase> refineMesh(const MeshBase* originalMesh, const SubdivisionOptions& opts)
	{
		PR_PROFILE_THIS;

		PR_ASSERT(opts.MaxLevel >= 1, "No refinement done!");

		Far::TopologyRefiner* refiner			= setupRefiner(originalMesh, opts);
		Far::StencilTable const* stencilTable	= setupStencilTable(refiner, Far::StencilTableFactory::INTERPOLATE_VERTEX);
		Far::StencilTable const* uvStencilTable = stencilTable;
		if ((originalMesh->features() & MF_HAS_UV) && opts.UVInterpolation != Far::StencilTableFactory::INTERPOLATE_VERTEX)
			uvStencilTable = setupStencilTable(refiner, opts.UVInterpolation);
		Far::StencilTable const* fvarStencilTable = (originalMesh->features() & MF_HAS_MATERIAL)
														? setupStencilTable(refiner, Far::StencilTableFactory::INTERPOLATE_FACE_VARYING)
														: nullptr;

		Far::TopologyLevel const& refLevel = refiner->GetLevel(refiner->GetMaxLevel());
		std::vector<uint32> new_indices(refLevel.GetNumFaceVertices());
		std::vector<uint8> new_vertsPerFace(refLevel.GetNumFaces());

		PR_LOG(L_INFO) << "Subdivision(L=" << opts.MaxLevel << "/" << refiner->GetMaxLevel() << ") [V=" << originalMesh->nodeCount() << " F=" << originalMesh->faceCount() << " I=" << originalMesh->indices().size()
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
		std::unique_ptr<MeshBase> refineMesh = std::make_unique<MeshBase>();

		refineMesh->setIndices(new_indices);
		refineMesh->setFaceVertexCount(new_vertsPerFace);
		refineVertexData(originalMesh, refineMesh, opts.ComputationMode, refiner, stencilTable);
		// TODO: Add normals based on limit
		refineMesh->buildNormals();

		if (originalMesh->features() & MF_HAS_UV)
			refineUVData(originalMesh, refineMesh, opts.ComputationMode, refiner, uvStencilTable);

		if (originalMesh->features() & MF_HAS_MATERIAL)
			refineMaterialData(originalMesh, refineMesh, opts.ComputationMode, refiner, fvarStencilTable);

		if (originalMesh->features() & MF_HAS_VELOCITY)
			refineVelocityData(originalMesh, refineMesh, opts.ComputationMode, refiner, stencilTable);

		// Remove buffers
		std::vector<uint32> null;
		fvarIndices.swap(null);

		delete refiner;
		if (stencilTable != uvStencilTable)
			delete uvStencilTable;
		delete stencilTable;
		if (fvarStencilTable)
			delete fvarStencilTable;

		return refineMesh;
	}

	std::shared_ptr<IEntity> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		std::string name	  = params.getString("name", "__unnamed__");
		std::string mesh_name = params.getString("mesh", "");

		std::vector<std::string> matNames = params.getStringArray("materials");

		std::vector<uint32> materials;
		for (std::string n : matNames) {
			std::shared_ptr<IMaterial> mat = ctx.Env->getMaterial(n);
			if (mat)
				materials.push_back(mat->id());
		}

		std::string emsName			   = params.getString("emission", "");
		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = ctx.Env->getEmission(emsName);
		if (ems)
			emsID = ems->id();

		SubdivisionOptions opts;
		opts.MaxLevel					  = std::max(1, (int)params.getInt("max_level", opts.MaxLevel));
		opts.Adaptive					  = params.getBool("adaptive", opts.Adaptive);
		std::string scheme				  = params.getString("scheme", "");
		std::string boundaryInterpolation = params.getString("boundary_interpolation", "");
		std::string uvInterpolation		  = params.getString("uv_interpolation", "");
		std::string fVarInterpolation	  = params.getString("fvar_interpolation", "");

		// Scheme
		std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
		if (scheme == "catmark")
			opts.Scheme = Sdc::SCHEME_CATMARK;
		else if (scheme == "bilinear")
			opts.Scheme = Sdc::SCHEME_BILINEAR;
		else if (scheme == "loop")
			opts.Scheme = Sdc::SCHEME_LOOP;

		// Boundary Interpolation
		std::transform(boundaryInterpolation.begin(), boundaryInterpolation.end(), boundaryInterpolation.begin(), ::tolower);
		if (boundaryInterpolation == "none")
			opts.BoundaryInterpolation = Sdc::Options::VTX_BOUNDARY_NONE;
		else if (boundaryInterpolation == "edge_only")
			opts.BoundaryInterpolation = Sdc::Options::VTX_BOUNDARY_EDGE_ONLY;
		else if (boundaryInterpolation == "edge_and_corner")
			opts.BoundaryInterpolation = Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER;

		// FVar Interpolation
		std::transform(fVarInterpolation.begin(), fVarInterpolation.end(), fVarInterpolation.begin(), ::tolower);
		if (fVarInterpolation == "none")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_NONE;
		else if (fVarInterpolation == "corners_only")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_CORNERS_ONLY;
		else if (fVarInterpolation == "corners_plus1")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_CORNERS_PLUS1;
		else if (fVarInterpolation == "corners_plus2")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_CORNERS_PLUS2;
		else if (fVarInterpolation == "boundaries")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_BOUNDARIES;
		else if (fVarInterpolation == "all")
			opts.FVarInterpolation = Sdc::Options::FVAR_LINEAR_ALL;

		// UV Interpolation
		std::transform(uvInterpolation.begin(), uvInterpolation.end(), uvInterpolation.begin(), ::tolower);
		if (uvInterpolation == "vertex")
			opts.UVInterpolation = Far::StencilTableFactory::INTERPOLATE_VERTEX;
		else if (uvInterpolation == "varying")
			opts.UVInterpolation = Far::StencilTableFactory::INTERPOLATE_VARYING;

		std::shared_ptr<Mesh> mesh = ctx.Env->getMesh(mesh_name);
		if (opts.MaxLevel < 1) {
			return std::make_shared<SubdivMeshEntity>(id, name,
													  mesh,
													  materials, emsID);
		}

		MeshBase* originalMesh = mesh->base_unsafe();
		if (!originalMesh)
			return nullptr;

		if (!originalMesh->isValid()) {
			PR_LOG(L_ERROR) << "Mesh " << mesh_name << " is invalid." << std::endl;
			return nullptr;
		}

		if (opts.Scheme == Sdc::SCHEME_LOOP) // Scheme Loop accepts triangle data only
			originalMesh->triangulate();

		std::unique_ptr<MeshBase> refinedMesh = refineMesh(originalMesh, opts);
		refinedMesh->triangulate();
		bool useCache				  = mesh->cache()->shouldCacheMesh(refinedMesh->nodeCount());
		std::shared_ptr<Mesh> newMesh = MeshFactory::create(mesh->name() + "_subdiv", std::move(refinedMesh),
														mesh->cache(), useCache);
		return std::make_shared<SubdivMeshEntity>(id, name,
												  newMesh,
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

PR_PLUGIN_INIT(PR::SubdivMeshEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)