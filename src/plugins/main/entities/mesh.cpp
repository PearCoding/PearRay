#include "mesh/Mesh.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "cache/ISerializeCachable.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "mesh/MeshFactory.h"

#include <boost/filesystem.hpp>

namespace PR {

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
		, mUseCache(false)
	{
	}
	virtual ~MeshEntity() {}

	inline void useCache(bool b) { mUseCache = b; }

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
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			uint32 id	  = extract(i, out.MaterialID);
			out.MaterialID = insert(i, out.MaterialID,
									id < mMaterials.size()
										? mMaterials.at(id)
										: 0);
		}
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mMesh->checkCollision(in_local, out);

		// out.FaceID is set inside mesh
		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.MaterialID  = out.MaterialID < mMaterials.size()
							 ? mMaterials.at(out.MaterialID)
							 : 0;
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
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

		pt.MaterialID = pt.MaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

private:
	int32 mLightID;
	std::vector<uint32> mMaterials;
	std::shared_ptr<Mesh> mMesh;
	bool mUseCache;
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