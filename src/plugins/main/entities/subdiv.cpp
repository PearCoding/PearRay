#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/GeometryPoint.h"
#include "math/Projection.h"
#include "math/SplitSample.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

namespace PR {

enum class SubdivisionMode {
	NoBoundary,
	SmoothBoundary,
	PinCorners,
	PinBoundary,
	PinAll
};

constexpr uint32 POSITION_SLOT = 0; // Vertex
constexpr uint32 UV_SLOT	   = 0; // Vertex-Attribute

struct SubdivParameters {
	uint32 TessellationRate;
	SubdivisionMode Mode;
	SubdivisionMode UVMode;
	bool UseCreases;
};

static inline void setMode(RTCGeometry geometry, unsigned int topologyID, SubdivisionMode mode)
{
	switch (mode) {
	case SubdivisionMode::NoBoundary:
		rtcSetGeometrySubdivisionMode(geometry, topologyID, RTC_SUBDIVISION_MODE_NO_BOUNDARY);
		break;
	default:
	case SubdivisionMode::SmoothBoundary:
		rtcSetGeometrySubdivisionMode(geometry, topologyID, RTC_SUBDIVISION_MODE_SMOOTH_BOUNDARY);
		break;
	case SubdivisionMode::PinCorners:
		rtcSetGeometrySubdivisionMode(geometry, topologyID, RTC_SUBDIVISION_MODE_PIN_CORNERS);
		break;
	case SubdivisionMode::PinBoundary:
		rtcSetGeometrySubdivisionMode(geometry, topologyID, RTC_SUBDIVISION_MODE_PIN_BOUNDARY);
		break;
	case SubdivisionMode::PinAll:
		rtcSetGeometrySubdivisionMode(geometry, topologyID, RTC_SUBDIVISION_MODE_PIN_ALL);
		break;
	}
}

// TODO: Add features like creases etc
class SubdivMesh {
public:
	explicit SubdivMesh(const std::shared_ptr<MeshBase>& mesh, const SubdivParameters& params)
		: mScene()
		, mBase(mesh)
		, mWasGenerated(false)
		, mParameters(params)
	{
		mFaceCount.resize(mesh->faceCount());
		if (mBase->isOnlyTriangular() || mBase->isOnlyQuadrangular()) {
			std::fill(mFaceCount.begin(), mFaceCount.end(), mBase->isOnlyTriangular() ? 3 : 4);
		} else {
			PR_OPT_LOOP
			for (size_t i = 0; i < mBase->faceCount(); ++i)
				mFaceCount[i] = mBase->faceVertexCount(i);
		}
	}

	~SubdivMesh()
	{
		rtcReleaseGeometry(mGeometry);
		rtcReleaseScene(mScene);
	}

	inline RTCScene generate(const RTCDevice& dev)
	{
		if (!mWasGenerated) {
			setupOriginal(dev);
			mWasGenerated = true;
		}

		return mScene;
	}

	inline std::tuple<Vector3f, Vector3f> interpolateTangent(uint32 primID, const Vector2f& param) const
	{
		RTCInterpolateArguments args;
		args.bufferType = RTC_BUFFER_TYPE_VERTEX;
		args.bufferSlot = POSITION_SLOT;
		args.geometry	= mGeometry;
		args.primID		= primID;
		args.u			= param(0);
		args.v			= param(1);
		args.valueCount = 3;

		RTC_ALIGN(16)
		float Pu[3];
		RTC_ALIGN(16)
		float Pv[3];
		args.P		 = nullptr;
		args.dPdu	 = Pu;
		args.dPdv	 = Pv;
		args.ddPdudu = nullptr;
		args.ddPdudv = nullptr;
		args.ddPdvdv = nullptr;

		rtcInterpolate(&args);

		return { Vector3f(Pu[0], Pu[1], Pu[2]), Vector3f(Pv[0], Pv[1], Pv[2]) };
	}

	inline Vector2f interpolateUV(uint32 primID, const Vector2f& param) const
	{
		RTCInterpolateArguments args;
		args.bufferType = RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE;
		args.bufferSlot = UV_SLOT;
		args.geometry	= mGeometry;
		args.primID		= primID;
		args.u			= param(0);
		args.v			= param(1);
		args.valueCount = 2;

		RTC_ALIGN(16)
		float P[2];
		args.P		 = P;
		args.dPdu	 = nullptr;
		args.dPdv	 = nullptr;
		args.ddPdudu = nullptr;
		args.ddPdudv = nullptr;
		args.ddPdvdv = nullptr;

		rtcInterpolate(&args);

		return Vector2f(P[0], P[1]);
	}

	inline MeshBase* base() const { return mBase.get(); }
	inline RTCScene scene() const { return mScene; }

private:
	inline void setupOriginal(const RTCDevice& dev)
	{
		mGeometry = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_SUBDIVISION);

		rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, mBase->vertices().data(), 0, sizeof(float) * 3, mBase->vertices().size() / 3);
		rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT, mBase->indices().data(), 0, sizeof(uint32), mBase->indices().size());
		rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_FACE, 0, RTC_FORMAT_UINT, mFaceCount.data(), 0, sizeof(uint32), mBase->faceCount());

		setMode(mGeometry, 0, mParameters.Mode);

		// UV
		if (mBase->features() & MeshFeature::UV) {
			rtcSetGeometryVertexAttributeCount(mGeometry, 1);
			rtcSetGeometryTopologyCount(mGeometry, 2);
			rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, UV_SLOT, RTC_FORMAT_FLOAT2, mBase->uvs().data(), 0, sizeof(float) * 2, mBase->uvs().size() / 2);
			rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_INDEX, 1 /* topologyID */, RTC_FORMAT_UINT, mBase->indices().data(), 0, sizeof(uint32), mBase->indices().size());
			rtcSetGeometryVertexAttributeTopology(mGeometry, UV_SLOT, 1);
			setMode(mGeometry, 1, mParameters.UVMode);
		}

		rtcSetGeometryTessellationRate(mGeometry, mParameters.TessellationRate);

		rtcCommitGeometry(mGeometry);

		mScene = rtcNewScene(dev);

		rtcAttachGeometry(mScene, mGeometry);

		rtcSetSceneFlags(mScene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST);
		rtcSetSceneBuildQuality(mScene, RTC_BUILD_QUALITY_HIGH);
		rtcCommitScene(mScene);
	}

	RTCScene mScene;
	RTCGeometry mGeometry;
	std::shared_ptr<MeshBase> mBase;
	bool mWasGenerated;

	const SubdivParameters mParameters;

	// Our implementation does not build a face count buffer but an optional index shift array.
	// However Embree requires one!
	std::vector<uint32> mFaceCount;
};

template <bool HasUV, bool CustomNormal>
class SubdivMeshEntity : public IEntity {
public:
	ENTITY_CLASS

	SubdivMeshEntity(const std::string& name, const Transformf& transform,
					 const std::shared_ptr<SubdivMesh>& mesh,
					 const std::vector<uint32>& materials,
					 uint32 lightID)
		: IEntity(lightID, name, transform)
		, mMaterials(materials)
		, mMesh(mesh)
		, mBoundingBox(mesh->base()->constructBoundingBox())
	{
	}
	virtual ~SubdivMeshEntity() {}

	std::string type() const override
	{
		return "mesh";
	}

	virtual float localSurfaceArea(uint32 id) const override
	{
		return mMesh->base()->surfaceArea(id, Eigen::Affine3f::Identity());
	}

	bool isCollidable() const override
	{
		return true;
	}

	float collisionCost() const override
	{
		return (float)mMesh->base()->faceCount();
	}

	virtual BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		RTCScene original = mMesh->generate(dev);

		RTCGeometry geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_INSTANCE);
		rtcSetGeometryInstancedScene(geom, original);

		const Transformf& M = transform();
		rtcSetGeometryTransform(geom, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, M.data());
		rtcCommitGeometry(geom);

		return GeometryRepr(geom);
	}

	// TODO: Better sampling pdf for sampleParameterPointPDF
	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		PR_PROFILE_THIS;
		const SplitSample2D split(rnd, 0, mMesh->base()->faceCount());
		const uint32 faceID = split.integral1();

		const Face face	  = mMesh->base()->getFace(faceID);
		const float pdf_a = 1.0f / (mMesh->base()->faceCount() * face.surfaceArea() * volumeScalefactor());

		Vector2f uv;
		if (!face.IsQuad)
			uv = Triangle::sample(Vector2f(split.uniform1(), split.uniform2()));
		else
			uv = Vector2f(split.uniform1(), split.uniform2());

		return EntitySamplePoint(transform() * face.interpolateVertices(uv), uv, faceID, pdf_a);
	}

	void provideGeometryPointLocal(const EntityGeometryQueryPoint& query,
								   GeometryPoint& pt) const
	{
		const Face face = mMesh->base()->getFace(query.PrimitiveID);
		if constexpr (CustomNormal) {
			pt.N = face.interpolateNormals(query.UV);
			if constexpr (HasUV)
				face.tangentFromUV(pt.N, pt.Nx, pt.Ny);
			else
				Tangent::unnormalized_frame(pt.N, pt.Nx, pt.Ny);
		} else {
			auto tuple = mMesh->interpolateTangent(query.PrimitiveID, query.UV);
			pt.Nx	   = std::get<0>(tuple);
			pt.Ny	   = std::get<1>(tuple);
			pt.N	   = pt.Nx.cross(pt.Ny);
		}

		if constexpr (HasUV)
			pt.UV = mMesh->interpolateUV(query.PrimitiveID, query.UV);
		else
			pt.UV = query.UV;

		pt.MaterialID = face.MaterialSlot < mMaterials.size() ? mMaterials.at(face.MaterialSlot) : PR_INVALID_ID;
	}

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// Local
		provideGeometryPointLocal(query, pt);

		// Global
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.PrimitiveID = query.PrimitiveID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = PR_INVALID_ID;
	}

private:
	const std::vector<uint32> mMaterials;
	const std::shared_ptr<SubdivMesh> mMesh;
	const BoundingBox mBoundingBox;
};

static inline SubdivisionMode strToMode(const std::string& str)
{
	std::string modeStr = str;
	std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(), ::tolower);
	if (modeStr == "no_boundary")
		return SubdivisionMode::NoBoundary;
	else if (modeStr == "pin_corners")
		return SubdivisionMode::PinCorners;
	else if (modeStr == "pin_boundary")
		return SubdivisionMode::PinBoundary;
	else if (modeStr == "pin_all")
		return SubdivisionMode::PinAll;
	else
		return SubdivisionMode::SmoothBoundary;
}

class SubdivMeshEntityPlugin : public IEntityPlugin {
public:
	std::unordered_map<MeshBase*, std::shared_ptr<SubdivMesh>> mOriginalMesh;

	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name	  = params.getString("name", "__unnamed__");
		std::string mesh_name = params.getString("mesh", "");

		const std::vector<uint32> materials = ctx.lookupMaterialIDArray(params.getParameter("materials"));
		const uint32 emsID					= ctx.lookupEmissionID(params.getParameter("emission"));
		const bool customNormal				= !params.getBool("smooth_normal", true);

		if (!ctx.hasMesh(mesh_name)) {
			PR_LOG(L_ERROR) << "Could not find a mesh named " << mesh_name << std::endl;
			return nullptr;
		} else {
			auto mesh = ctx.getMesh(mesh_name);
			std::shared_ptr<SubdivMesh> mesh_p;
			if (mOriginalMesh.count(mesh.get()) > 0) {
				mesh_p = mOriginalMesh.at(mesh.get());
			} else {
				SubdivParameters sp;
				sp.TessellationRate = params.getUInt("tessellation", 4);
				sp.Mode				= strToMode(params.getString("mode", ""));
				sp.UVMode			= strToMode(params.getString("uv_mode", ""));
				sp.UseCreases		= params.getBool("creases", true); // Only if available (TODO)

				mesh_p					  = std::make_shared<SubdivMesh>(mesh, sp);
				mOriginalMesh[mesh.get()] = mesh_p;
			}

			if (customNormal) {
				if (mesh->features() & MeshFeature::UV)
					return std::make_shared<SubdivMeshEntity<true, true>>(name, ctx.transform(),
																		  mesh_p,
																		  materials, emsID);
				else
					return std::make_shared<SubdivMeshEntity<false, true>>(name, ctx.transform(),
																		   mesh_p,
																		   materials, emsID);
			} else {
				if (mesh->features() & MeshFeature::UV)
					return std::make_shared<SubdivMeshEntity<true, false>>(name, ctx.transform(),
																		   mesh_p,
																		   materials, emsID);
				else
					return std::make_shared<SubdivMeshEntity<false, false>>(name, ctx.transform(),
																			mesh_p,
																			materials, emsID);
			}
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "subdiv", "subdivision", "subdiv_mesh", "subdivmesh", "smooth_mesh" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Subdivision Mesh Entity", "A mesh entity with subdivision")
			.Identifiers(getNames())
			.Inputs()
			.MeshReference("mesh", "Mesh")
			.MaterialReferenceV({ "material", "materials" }, "Material")
			.EmissionReference("emission", "Emission", true)
			.UInt("tessellation", "Tesselation Rate", 4)
			.Option("mode", "Subdivision mode", "smooth_boundary", { "no_boundary", "smooth_boundary", "pin_corners", "pin_boundary", "pin_all" })
			.Option("uv_mode", "Subdivision mode for uv coordinates", "smooth_boundary", { "no_boundary", "smooth_boundary", "pin_corners", "pin_boundary", "pin_all" })
			.Bool("creases", "Use creases if available", true)
			.Bool("smooth_normal", "Use calculated smooth normals", true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SubdivMeshEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)