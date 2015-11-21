#pragma once

#include "Entity.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	class FacePoint;
	class Material;
	class Random;
	class Ray;
	class Renderer;
	class PR_LIB GeometryEntity : public Entity
	{
	public:
		GeometryEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~GeometryEntity();

		virtual bool isLight() const = 0;

		virtual bool isCollidable() const;
		virtual BoundingBox localBoundingBox() const;
		virtual BoundingBox worldBoundingBox() const;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint);

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer);

		virtual FacePoint getRandomFacePoint(Random& random) const = 0;
	};
}