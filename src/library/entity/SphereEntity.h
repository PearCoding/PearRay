#pragma once

#include "GeometryEntity.h"

namespace PR
{
	class Material;
	class PR_LIB SphereEntity : public GeometryEntity
	{
	public:
		SphereEntity(const std::string& name, float r, Entity* parent = nullptr);
		virtual ~SphereEntity();

		void setRadius(float f);
		float radius() const;

		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox boundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;
	private:
		float mRadius;
		Material* mMaterial;
	};
}