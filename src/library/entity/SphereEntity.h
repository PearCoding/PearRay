#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Material;
	class PR_LIB SphereEntity : public RenderEntity
	{
	public:
		SphereEntity(const std::string& name, float r, Entity* parent = nullptr);
		virtual ~SphereEntity();

		virtual std::string type() const;

		virtual bool isLight() const override;

		void setMaterial(Material* m);
		Material* material() const;

		void setRadius(float f);
		float radius() const;
		
		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, SamplePoint& collisionPoint) override;

		virtual SamplePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const;
	private:
		float mRadius;
		Material* mMaterial;
	};
}