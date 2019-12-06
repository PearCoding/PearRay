#include "geometry/Sphere.h"
#include "Environment.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "geometry/GeometryPoint.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"

namespace PR {
constexpr float P = 1.6075f;

class SphereEntity : public IEntity {
public:
	ENTITY_CLASS

	SphereEntity(uint32 id, const std::string& name, float r,
				 int32 matID, int32 lightID)
		: IEntity(id, name)
		, mSphere(r)
		, mMaterialID(matID)
		, mLightID(lightID)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~SphereEntity() {}

	std::string type() const override
	{
		return "sphere";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || mMaterialID < 0 || id == (uint32)mMaterialID) {
			Eigen::Matrix3f sca;
			transform().computeRotationScaling((Eigen::Matrix3f*)nullptr, &sca);

			const auto s = flags() & EF_LocalArea ? Vector3f(1, 1, 1) : sca.diagonal();

			const float a = s(0) * mSphere.radius();
			const float b = s(1) * mSphere.radius();
			const float c = s(2) * mSphere.radius();

			// Knud Thomsen’s Formula
			const float t = (std::pow(a * b, P) + std::pow(a * c, P) + std::pow(b * c, P)) / 3;
			return 4 * PR_PI * std::pow(t, 1 / P);
		} else {
			return 0;
		}
	}

	bool isCollidable() const override
	{
		return mMaterialID > 0 && mSphere.radius() >= PR_EPSILON;
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return BoundingBox(Vector3f(mSphere.radius(), mSphere.radius(), mSphere.radius()),
						   Vector3f(-mSphere.radius(), -mSphere.radius(), -mSphere.radius()));
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mSphere.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(),
													   in);
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());
		mSphere.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(),
													   in);
		out.EntityID	= id();
		out.FaceID		= 0;
		out.MaterialID  = mMaterialID;
	}

	Vector3f pickRandomParameterPoint(const Vector3f& view, const Vector2f& rnd,
									  uint32& faceID, float& pdf) const override
	{
		pdf	= mPDF_Cache;
		faceID = 0;

		Vector3f kv = invTransform().linear() * view;
		Vector3f n  = mSphere.normalPoint(rnd(0), rnd(1));

		Vector2f uv = rnd;
		if (kv.dot(n) > 0)
			uv = Spherical::uv_from_normal(Vector3f(-n));

		return Vector3f(uv(0), uv(1), 0);
	}

	void provideGeometryPoint(const Vector3f&, uint32, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		pt.P = transform() * mSphere.surfacePoint(parameter[0], parameter[1]);
		pt.N = normalMatrix() * mSphere.normalPoint(parameter[0], parameter[1]);
		pt.N.normalize();

		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.UVW		  = parameter;
		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	void beforeSceneBuild() override
	{
		IEntity::beforeSceneBuild();

		const float area = surfaceArea(0);
		// We only consider view side of the sphere,
		// therefore only half of the surface is considered in sampling.
		mPDF_Cache = 0.5f * (area > PR_EPSILON ? 1.0f / area : 0);
	}

private:
	Sphere mSphere;
	int32 mMaterialID;
	int32 mLightID;

	float mPDF_Cache;
};

class SphereEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const auto& reg  = env.registry();
		std::string name = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		float r			 = reg.getForObject<float>(RG_ENTITY, uuid, "radius", 1.0f);

		std::string emsName = reg.getForObject<std::string>(RG_ENTITY, uuid, "emission", "");
		std::string matName = reg.getForObject<std::string>(RG_ENTITY, uuid, "material", "");

		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = env.getMaterial(matName);
		if (mat)
			matID = mat->id();

		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = ems->id();

		return std::make_shared<SphereEntity>(id, name, r, matID, emsID);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "sphere" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SphereEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)