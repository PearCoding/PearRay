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

class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(uint32 id, const std::string& name,
			   const std::shared_ptr<MeshBase>& mesh,
			   const std::vector<uint32>& materials,
			   int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
		, mMaterials(materials)
		, mMesh(mesh)
		, mBoundingBox(mesh->constructBoundingBox())
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
		return mMesh->surfaceArea(id, Eigen::Affine3f::Identity());
	}

	bool isCollidable() const override
	{
		return true;
	}

	float collisionCost() const override
	{
		return (float)mMesh->faceCount();
	}

	BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		auto geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_TRIANGLE);
		return GeometryRepr(geom);
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		PR_PROFILE_THIS;
		SplitSample2D split(rnd, 0, mMesh->faceCount());
		faceID = split.integral1();

		Face face = mMesh->getFace(faceID);
		pdf		  = 1.0f / (mMesh->faceCount() * face.surfaceArea());

		return Vector3f(split.uniform1(), split.uniform2(), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32 faceID, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// Local
		Vector2f uv;
		Face face = mMesh->getFace(faceID);
		face.interpolate(Vector2f(parameter[0], parameter[1]), pt.P, pt.N, uv);

		if (mMesh->features() & MF_HAS_UV) // Could be amortized by two types of mesh entities!
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
	std::shared_ptr<MeshBase> mMesh;
	const BoundingBox mBoundingBox;
};

class MeshEntityPlugin : public IEntityPlugin {
public:
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
			return std::make_shared<MeshEntity>(id, name,
												mesh,
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