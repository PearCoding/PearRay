#include "geometry/Disk.h"
#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"

namespace PR {

class DiskEntity : public IEntity {
public:
	ENTITY_CLASS

	DiskEntity(uint32 id, const std::string& name,
			   float radius,
			   int32 matID, int32 lightID)
		: IEntity(id, name)
		, mDisk(radius)
		, mMaterialID(matID)
		, mLightID(lightID)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~DiskEntity() {}

	std::string type() const override
	{
		return "disk";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || mMaterialID < 0 || id == (uint32)mMaterialID)
			return mDisk.surfaceArea();
		else
			return 0;
	}

	bool isCollidable() const override
	{
		return mDisk.isValid();
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return mDisk.toBoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		mDisk.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		mDisk.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.FaceID		= 0;
		out.MaterialID  = mMaterialID;
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		pdf	= mPDF_Cache;
		faceID = 0;
		return Vector3f(rnd(0), rnd(1), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		float u = parameter[0];
		float v = parameter[1];

		pt.P = transform() * mDisk.surfacePoint(u, v);
		pt.N = normalMatrix() * mDisk.normal();

		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UVW		  = Vector3f(u, v, 0);
		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		const float area = surfaceArea(0);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);
	}

private:
	Disk mDisk;
	int32 mMaterialID;
	int32 mLightID;

	float mPDF_Cache;
};

class DiskEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const SceneLoadContext& ctx)
	{
		const auto& reg = ctx.Env->registry();

		std::string name = reg.getForObject<std::string>(RG_ENTITY, uuid, "name",
														 "__unnamed__");
		float radius	 = reg.getForObject<float>(RG_ENTITY, uuid, "radius", 1);

		std::string emsName = reg.getForObject<std::string>(RG_ENTITY, uuid, "emission", "");
		std::string matName = reg.getForObject<std::string>(RG_ENTITY, uuid, "material", "");

		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = ctx.Env->getMaterial(matName);
		if (mat)
			matID = mat->id();

		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = ctx.Env->getEmission(emsName);
		if (ems)
			emsID = ems->id();

		return std::make_shared<DiskEntity>(id, name, radius, matID, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "disk" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DiskEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)