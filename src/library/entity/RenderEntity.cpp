#include "RenderEntity.h"

#include "material/Material.h"

namespace PR
{
	RenderEntity::RenderEntity(const std::string& name, Entity* parent) :
		Entity(name, parent), mMaterial(nullptr)
	{
	}

	RenderEntity::~RenderEntity()
	{
	}

	bool RenderEntity::isRenderable() const
	{
		return true;
	}

	bool RenderEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	void RenderEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* RenderEntity::material() const
	{
		return mMaterial;
	}

	bool RenderEntity::isCollidable() const
	{
		return false;
	}

	BoundingBox RenderEntity::localBoundingBox() const
	{
		return BoundingBox();
	}

	BoundingBox RenderEntity::worldBoundingBox() const
	{
		float sc = scale();
		const PM::quat rot = rotation();
		const PM::vec3 pos = position();

		BoundingBox bx = localBoundingBox();

		if (!PM::pm_IsNearlyEqual(PM::pm_IdentityQuat(), rot, PM::pm_FillVector(PM_EPSILON)))
			sc *= 1.41421356f;

		PM::vec3 upper = PM::pm_Add(pos, PM::pm_Scale(bx.upperBound(), sc));
		PM::vec3 lower = PM::pm_Add(pos, PM::pm_Scale(bx.lowerBound(), sc));

		return BoundingBox(upper, lower);
	}

	bool RenderEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		return false;
	}
}