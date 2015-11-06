#pragma once

#include "Entity.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	class FacePoint;
	class Ray;
	class Renderer;
	class PR_LIB GeometryEntity : public Entity
	{
	public:
		GeometryEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~GeometryEntity();

		virtual bool isCollidable() const;
		virtual BoundingBox boundingBox() const;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint);

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer);
	};
}