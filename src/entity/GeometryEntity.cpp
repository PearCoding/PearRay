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

	BoundingBox GeometryEntity::boundingBox() const
	{
		return BoundingBox(PM::pm_Set(0, 0, 0), PM::pm_Set(0, 0, 0));
	}

	bool GeometryEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		return false;
	}

	void GeometryEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
	}
}