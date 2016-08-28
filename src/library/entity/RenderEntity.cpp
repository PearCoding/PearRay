#include "RenderEntity.h"

#include "material/Material.h"

namespace PR
{
	RenderEntity::RenderEntity(const std::string& name, Entity* parent) :
		Entity(name, parent)
	{
	}

	RenderEntity::~RenderEntity()
	{
	}

	bool RenderEntity::isRenderable() const
	{
		return true;
	}

	bool RenderEntity::isCollidable() const
	{
		return false;
	}
	
	float RenderEntity::collisionCost() const
	{
		return 1;
	}

	BoundingBox RenderEntity::localBoundingBox() const
	{
		return BoundingBox();
	}

	BoundingBox RenderEntity::worldBoundingBox() const
	{
		const PM::quat rot = rotation();

		BoundingBox bx = localBoundingBox();

		float sc = 1;
		if (!PM::pm_IsNearlyEqual(PM::pm_IdentityQuat(), rot, PM::pm_FillVector(PM_EPSILON)))
			sc = 1.41421356f;

		PM::vec3 upper = PM::pm_Multiply(matrix(), PM::pm_Scale(bx.upperBound(), sc));
		PM::vec3 lower = PM::pm_Multiply(matrix(), PM::pm_Scale(bx.lowerBound(), sc));

		return BoundingBox(upper, lower);
	}

	bool RenderEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint, float& t)
	{
		return false;
	}
}