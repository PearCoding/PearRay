#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"

#include "geometry/TriMesh.h"

#include "math/Projection.h"
#include "registry/Registry.h"

namespace PR {

class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(uint32 id, const std::string& name, float r)
		: IEntity(id, name)
		, mRadius(r)
		, mMaterialID(0)
		, mLightID(0)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~MeshEntity() {}

	std::string type() const override
	{
		return "mesh";
	}

	bool isLight() const override
	{
		return mLightID != 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || id == mMaterialID) {
			Eigen::Matrix3f sca;
			transform().computeRotationScaling((Eigen::Matrix3f*)nullptr, &sca);

			const auto s = flags() & EF_LocalArea ? Vector3f(1, 1, 1) : sca.diagonal();

			const float a = s(0) * mRadius;
			const float b = s(1) * mRadius;
			const float c = s(2) * mRadius;

			// Knud Thomsen’s Formula
			const float t = (std::pow(a * b, P) + std::pow(a * c, P) + std::pow(b * c, P)) / 3;
			return 4 * PR_PI * std::pow(t, 1 / P);
		} else {
			return 0;
		}
	}

	void setMaterialID(uint32 m)
	{
		mMaterialID = m;
	}

	uint32 materialID() const
	{
		return mMaterialID;
	}

	bool isCollidable() const override
	{
		return mRadius >= PR_EPSILON;
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return BoundingBox(Vector3f(mRadius, mRadius, mRadius),
						   Vector3f(-mRadius, -mRadius, -mRadius));
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());
		mSphere_Cache.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance, transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());
		mSphere_Cache.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance, transform().matrix(), in);
		out.EntityID	= id();
		out.FaceID		= 0;
		out.MaterialID  = mMaterialID;
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);

		const float area = surfaceArea(0);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);

		mSphere_Cache = Sphere(Vector3f(0, 0, 0), mRadius);
	}

private:
	float mRadius;
	uint32 mMaterialID;
	uint32 mLightID;

	Sphere mSphere_Cache;
	float mPDF_Cache;
};

class MeshEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Registry& reg)
	{
		std::string name = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		float r			 = reg.getForObject<float>(RG_ENTITY, uuid, "radius", 1.0f);
		return std::make_shared<SphereEntity>(id, name, r);
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

PR_PLUGIN_INIT(PR::SphereEntityFactory, "ent_sphere", PR_PLUGIN_VERSION)