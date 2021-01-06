#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/SplitSample.h"
#include "mesh/MeshBase.h"

#include <filesystem>

namespace PR {

class Mesh {
public:
	explicit Mesh(const std::shared_ptr<MeshBase>& mesh)
		: mScene()
		, mBase(mesh)
		, mWasGenerated(false)
	{
	}

	~Mesh()
	{
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

	inline MeshBase* base() const { return mBase.get(); }
	inline RTCScene scene() const { return mScene; }

private:
	inline void setupOriginal(const RTCDevice& dev)
	{
		RTCGeometry geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_TRIANGLE);

		// TODO: Make sure the internal mesh buffer is proper aligned at the end
		rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, mBase->vertices().data(), 0, sizeof(float) * 3, mBase->vertices().size() / 3);
		rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, mBase->indices().data(), 0, sizeof(uint32) * 3, mBase->indices().size() / 3);
		rtcCommitGeometry(geom);

		mScene = rtcNewScene(dev);

		rtcAttachGeometry(mScene, geom);
		rtcReleaseGeometry(geom);

		rtcSetSceneFlags(mScene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST | RTC_SCENE_FLAG_CONTEXT_FILTER_FUNCTION);
		rtcSetSceneBuildQuality(mScene, RTC_BUILD_QUALITY_HIGH);
		rtcCommitScene(mScene);
	}

	RTCScene mScene;
	std::shared_ptr<MeshBase> mBase;
	bool mWasGenerated;
};

template <bool HasUV>
class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(const std::string& name, const Transformf& transform,
			   const std::shared_ptr<Mesh>& mesh,
			   const std::vector<uint32>& materials,
			   uint32 lightID)
		: IEntity(lightID, name, transform)
		, mMaterials(materials)
		, mMesh(mesh)
		, mBoundingBox(mesh->base()->constructBoundingBox())
	{
	}
	virtual ~MeshEntity() {}

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

		const Vector2f uv = Triangle::sample(Vector2f(split.uniform1(), split.uniform2()));

		return EntitySamplePoint(transform() * face.interpolateVertices(uv), uv, faceID, pdf_a);
	}

	// UV variant
	template <bool UV = HasUV>
	inline typename std::enable_if<UV, void>::type
	provideGeometryPoint2(const EntityGeometryQueryPoint& query,
						  GeometryPoint& pt) const
	{
		Face face	= mMesh->base()->getFace(query.PrimitiveID);
		pt.N		= face.interpolateNormals(query.UV);
		Vector2f uv = face.interpolateUVs(query.UV);

		Tangent::unnormalized_frame(pt.N, pt.Nx, pt.Ny);
		//face.tangentFromUV(pt.N, pt.Nx, pt.Ny);
		pt.UV = uv;

		pt.MaterialID = face.MaterialSlot < mMaterials.size() ? mMaterials.at(face.MaterialSlot) : PR_INVALID_ID;
	}

	// Non UV variant
	template <bool UV = HasUV>
	inline typename std::enable_if<!UV, void>::type
	provideGeometryPoint2(const EntityGeometryQueryPoint& query,
						  GeometryPoint& pt) const
	{
		Face face = mMesh->base()->getFace(query.PrimitiveID);
		pt.N	  = face.interpolateNormals(query.UV);

		Tangent::unnormalized_frame(pt.N, pt.Nx, pt.Ny);
		pt.UV = query.UV;

		pt.MaterialID = face.MaterialSlot < mMaterials.size() ? mMaterials.at(face.MaterialSlot) : PR_INVALID_ID;
	}

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// Local
		provideGeometryPoint2(query, pt);

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
	std::vector<uint32> mMaterials;
	std::shared_ptr<Mesh> mMesh;
	const BoundingBox mBoundingBox;
};

class MeshEntityPlugin : public IEntityPlugin {
public:
	std::unordered_map<MeshBase*, std::shared_ptr<Mesh>> mOriginalMesh;

	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name	  = params.getString("name", "__unnamed__");
		std::string mesh_name = params.getString("mesh", "");

		const std::vector<uint32> materials = ctx.lookupMaterialIDArray(params.getParameter("materials"));
		const uint32 emsID					= ctx.lookupEmissionID(params.getParameter("emission"));

		if (!ctx.hasMesh(mesh_name)) {
			PR_LOG(L_ERROR) << "Could not find a mesh named " << mesh_name << std::endl;
			return nullptr;
		} else {
			auto mesh = ctx.getMesh(mesh_name);
			std::shared_ptr<Mesh> mesh_p;
			if (mOriginalMesh.count(mesh.get()) > 0) {
				mesh_p = mOriginalMesh.at(mesh.get());
			} else {
				mesh_p					  = std::make_shared<Mesh>(mesh);
				mOriginalMesh[mesh.get()] = mesh_p;
			}

			if (mesh->features() & MeshFeature::UV)
				return std::make_shared<MeshEntity<true>>(name, ctx.transform(),
														  mesh_p,
														  materials, emsID);
			else
				return std::make_shared<MeshEntity<false>>(name, ctx.transform(),
														   mesh_p,
														   materials, emsID);
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "mesh" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Plane Entity", "A solid plane")
			.Identifiers(getNames())
			.Inputs()
			.MeshReference("mesh", "Mesh")
			.MaterialReferenceV({ "material", "materials" }, "Material")
			.EmissionReference("emission", "Emission", true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MeshEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)