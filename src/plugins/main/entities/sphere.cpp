#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"

#include "geometry/Sphere.h"

#include "material/IMaterial.h"
#include "math/Projection.h"
#include "registry/Registry.h"

namespace PR {
constexpr float P = 1.6075f;

class ILight;

class SphereEntity : public IEntity {
public:
	ENTITY_CLASS

	SphereEntity(uint32 id, const std::string& name, float r)
		: IEntity(id, name)
		, mRadius(r)
		, mMaterial(nullptr)
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
		return mLight != nullptr;
	}

	float surfaceArea(IMaterial* m) const override
	{
		if (!m || m == mMaterial.get()) {
			Eigen::Matrix3f sca;
			transform().computeRotationScaling((Eigen::Matrix3f*)nullptr, &sca);

			const auto s = flags() & EF_LocalArea ? Eigen::Vector3f(1, 1, 1) : sca.diagonal();

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

	void setMaterial(const std::shared_ptr<IMaterial>& m)
	{
		mMaterial = m;
	}

	std::shared_ptr<IMaterial> material() const
	{
		return mMaterial;
	}

	void setRadius(float f)
	{
		mRadius = f;
	}

	float radius() const
	{
		return mRadius;
	}

	bool isCollidable() const override
	{
		return mMaterial && mMaterial->canBeShaded() && mRadius >= PR_EPSILON;
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return BoundingBox(Eigen::Vector3f(mRadius, mRadius, mRadius),
						   Eigen::Vector3f(-mRadius, -mRadius, -mRadius));
	}

	void checkCollision(const CollisionInput& in, CollisionOutput& out) const override
	{
		// TODO
		out.EntityID   = simdpp::make_uint(id());
		out.FaceID	 = simdpp::make_uint(0);
		out.MaterialID = simdpp::make_uint(mMaterial->id());
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);

		const float area = surfaceArea(nullptr);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);

		if (mMaterial)
			mMaterial->freeze(context);
	}

private:
	float mRadius;
	std::shared_ptr<IMaterial> mMaterial;
	std::shared_ptr<ILight> mLight;

	float mPDF_Cache;
};

class SphereEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Registry& reg)
	{
		std::string name = reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		float r			 = reg.getForObject<float>(RG_ENTITY, uuid, "radius", 1.0f);
		return std::make_shared<SphereEntity>(id, name, r);
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

PR_PLUGIN_INIT(PR::SphereEntityFactory, "sphere", "1.0")