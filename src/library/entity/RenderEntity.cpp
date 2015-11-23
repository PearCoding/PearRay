#include "RenderEntity.h"

namespace PR
{
	RenderEntity::RenderEntity(const std::string& name, Entity* parent) :
		Entity(name, parent)
	{
	}

	RenderEntity::~RenderEntity()
	{
	}

	uint32 RenderEntity::maxLightSamples() const
	{
		return 0;
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
		BoundingBox bx = localBoundingBox();

		PM::mat m = matrix();
		PM::vec3 upper = PM::pm_Multiply(m, bx.upperBound());
		PM::vec3 lower = PM::pm_Multiply(m, bx.lowerBound());

		// Really this way?
		if (!PM::pm_IsLess(upper, lower))
		{
			return BoundingBox(upper, lower);
		}
		else
		{
			return BoundingBox(lower, upper);
		}
	}

	bool RenderEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		return false;
	}

	void RenderEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
	}
}