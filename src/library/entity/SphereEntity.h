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
		
		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t) override;

		virtual FacePoint getRandomFacePoint(Sampler& sampler, Random& random) const;
	private:
		float mRadius;
	};
}