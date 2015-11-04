#pragma once

#include "GeometryEntity.h"

namespace PR
{
	class Material;
	class SphereEntity : public GeometryEntity
	{
	public:
		SphereEntity(const std::string& name, float r, Entity* parent = nullptr);
		virtual ~SphereEntity();

		void setRadius(float f);
		float radius() const;

		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const;
		virtual BoundingBox boundingBox() const;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint);

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer);
	private:
		float mRadius;
		Material* mMaterial;
	};
}