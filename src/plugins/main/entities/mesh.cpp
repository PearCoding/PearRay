#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "cache/ISerializeCachable.h"
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
	Mesh(const std::shared_ptr<MeshBase>& mesh)
		: mBase(mesh)
		, mWasGenerated(false)
	{
	}

	~Mesh()
	{
		rtcReleaseScene(mScene);
	}

	RTCScene generate(const RTCDevice& dev)
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

	MeshEntity(uint32 id, const std::string& name,
			   const std::shared_ptr<Mesh>& mesh,
			   const std::vector<uint32>& materials,
			   uint32 lightID)
		: IEntity(id, lightID, name)
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

	float localSurfaceArea(uint32 id) const override
	{
		return mMesh->base()->surfaceArea(id == PR_INVALID_ID ? 0 : id, Eigen::Affine3f::Identity());
	}

	bool isCollidable() const override
	{
		return true;
	}

	float collisionCost() const override
	{
		return (float)mMesh->base()->faceCount();
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		RTCScene original = mMesh->generate(dev);

		RTCGeometry geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_INSTANCE);
		rtcSetGeometryInstancedScene(geom, original);

		const auto M = transform();
		rtcSetGeometryTransform(geom, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, M.data());
		rtcCommitGeometry(geom);

		return GeometryRepr(geom);
	}

	// TODO: Better sampling pdf for sampleParameterPointPDF
	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		PR_PROFILE_THIS;
		SplitSample2D split(rnd, 0, mMesh->base()->faceCount());
		uint32 faceID = split.integral1();

		Face face = mMesh->base()->getFace(faceID);
		float pdf = 1.0f / (mMesh->base()->faceCount() * face.surfaceArea() * volumeScalefactor());

		Vector2f uv = Triangle::sample(Vector2f(split.uniform1(), split.uniform2()));

		return EntitySamplePoint(transform() * face.interpolateVertices(uv), uv, faceID, EntitySamplePDF{ pdf, true });
	}

	// UV variant
	template <bool UV = HasUV>
	inline typename std::enable_if<UV, void>::type
	provideGeometryPoint2(const EntityGeometryQueryPoint& query,
						  GeometryPoint& pt) const
	{
		// pt.P is not populated, as we will use the query position directly

		Face face	= mMesh->base()->getFace(query.PrimitiveID);
		pt.N		= face.interpolateNormals(query.UV);
		Vector2f uv = face.interpolateUVs(query.UV);

		face.tangentFromUV(pt.N, pt.Nx, pt.Ny);
		pt.UV = uv;

		pt.MaterialID = face.MaterialSlot < mMaterials.size() ? mMaterials.at(face.MaterialSlot) : PR_INVALID_ID;
	}

	// Non UV variant
	template <bool UV = HasUV>
	inline typename std::enable_if<!UV, void>::type
	provideGeometryPoint2(const EntityGeometryQueryPoint& query,
						  GeometryPoint& pt) const
	{
		// pt.P is not populated, as we will use the query position directly

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

		pt.EntityID	   = id();
		pt.PrimitiveID = query.PrimitiveID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

private:
	std::vector<uint32> mMaterials;
	std::shared_ptr<Mesh> mMesh;
	const BoundingBox mBoundingBox;
};

class MeshEntityPlugin : public IEntityPlugin {
public:
	std::unordered_map<MeshBase*, std::shared_ptr<Mesh>> mOriginalMesh;

	std::shared_ptr<IEntity> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name	  = params.getString("name", "__unnamed__");
		std::string mesh_name = params.getString("mesh", "");

		std::vector<std::string> matNames = params.getStringArray("materials");

		std::vector<uint32> materials;
		for (std::string n : matNames) {
			std::shared_ptr<IMaterial> mat = ctx.environment()->getMaterial(n);
			if (mat)
				materials.push_back(mat->id());
		}

		std::string emsName			   = params.getString("emission", "");
		uint32 emsID				   = PR_INVALID_ID;
		std::shared_ptr<IEmission> ems = ctx.environment()->getEmission(emsName);
		if (ems)
			emsID = ems->id();

		if (!ctx.environment()->hasMesh(mesh_name))
			return nullptr;
		else {
			auto mesh = ctx.environment()->getMesh(mesh_name);
			std::shared_ptr<Mesh> mesh_p;
			if (mOriginalMesh.count(mesh.get()) > 0) {
				mesh_p = mOriginalMesh.at(mesh.get());
			} else {
				mesh_p					  = std::make_shared<Mesh>(mesh);
				mOriginalMesh[mesh.get()] = mesh_p;
			}

			if (mesh->features() & MF_HAS_UV)
				return std::make_shared<MeshEntity<true>>(id, name,
														  mesh_p,
														  materials, emsID);
			else
				return std::make_shared<MeshEntity<false>>(id, name,
														   mesh_p,
														   materials, emsID);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "mesh" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MeshEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)