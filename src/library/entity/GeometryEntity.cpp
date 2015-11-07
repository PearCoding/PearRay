#include "GeometryEntity.h"

namespace PR
{
	GeometryEntity::GeometryEntity(const std::string& name, Entity* parent) :
		Entity(name, parent)
	{
	}

	GeometryEntity::~GeometryEntity()
	{
	}

	bool GeometryEntity::isCollidable() const
	{
		return false;
	}

	BoundingBox GeometryEntity::localBoundingBox() const
	{
		return BoundingBox();
	}

	BoundingBox GeometryEntity::worldBoundingBox() const
	{
		BoundingBox bx = localBoundingBox();

		PM::mat m = matrix();
		PM::vec3 upper = PM::pm_Multiply(m, bx.upperBound());
		PM::vec3 lower = PM::pm_Multiply(m, bx.lowerBound());

		// Really this way?
		if (PM::pm_IsGreaterOrEqual(upper, lower))
		{
			return BoundingBox(upper, lower);
		}
		else
		{
			return BoundingBox(lower, upper);
		}
	}

	bool GeometryEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		return false;
	}

	void GeometryEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
	}
}