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
#include "mesh/MeshBase.h"
#include "sampler/SplitSample.h"

#include <boost/filesystem.hpp>

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

		rtcSetSceneFlags(mScene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST);
		rtcSetSceneBuildQuality(mScene, RTC_BUILD_QUALITY_HIGH);
		rtcCommitScene(mScene);
	}

	RTCScene mScene;
	std::shared_ptr<MeshBase> mBase;
	bool mWasGenerated;
};

class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(uint32 id, const std::string& name,
			   const std::shared_ptr<Mesh>& mesh,
			   const std::vector<uint32>& materials,
			   int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
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

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
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

	EntityRandomPoint pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd) const override
	{
		PR_PROFILE_THIS;
		SplitSample2D split(rnd, 0, mMesh->base()->faceCount());
		uint32 faceID = split.integral1();

		Face face = mMesh->base()->getFace(faceID);
		float pdf = 1.0f / (mMesh->base()->faceCount() * face.surfaceArea());

		Vector2f uv = Vector2f(split.uniform1(), split.uniform2());

		return EntityRandomPoint(face.interpolateVertices(uv), uv, faceID, pdf);
	}

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// Local
		Vector2f uv;
		Face face = mMesh->base()->getFace(query.PrimitiveID);
		face.interpolate(Vector2f(query.UV[0], query.UV[1]), pt.P, pt.N, uv);

		if (mMesh->base()->features() & MF_HAS_UV) // Could be amortized by two types of mesh entities!
			face.tangentFromUV(pt.N, pt.Nx, pt.Ny);
		else
			Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.UVW = Vector3f(uv(0), uv(1), 0);

		// Global
		pt.P  = transform() * pt.P;
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = pt.MaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

private:
	const int32 mLightID;
	std::vector<uint32> mMaterials;
	std::shared_ptr<Mesh> mMesh;
	const BoundingBox mBoundingBox;
};

class MeshEntityPlugin : public IEntityPlugin {
public:
	std::unordered_map<MeshBase*, std::shared_ptr<Mesh>> mOriginalMesh;

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

		if (!ctx.Env->hasMesh(mesh_name))
			return nullptr;
		else {
			auto mesh = ctx.Env->getMesh(mesh_name);
			std::shared_ptr<Mesh> mesh_p;
			if (mOriginalMesh.count(mesh.get()) > 0) {
				mesh_p = mOriginalMesh.at(mesh.get());
			} else {
				mesh_p					  = std::make_shared<Mesh>(mesh);
				mOriginalMesh[mesh.get()] = mesh_p;
			}

			return std::make_shared<MeshEntity>(id, name,
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