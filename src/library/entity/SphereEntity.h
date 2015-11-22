#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Material;

	/* Does not support scale! */
	class PR_LIB SphereEntity : public RenderEntity
	{
	public:
		SphereEntity(const std::string& name, float r, Entity* parent = nullptr);
		virtual ~SphereEntity();

		virtual std::string type() const;

		void setRadius(float f);
		float radius() const;

		bool isLight() const;
		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;

		virtual FacePoint getRandomFacePoint(Random& random) const;
	private:
		float mRadius;
		Material* mMaterial;
	};
}