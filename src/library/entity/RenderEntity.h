#pragma once

#include "Entity.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	class Material;
	class Random;
	class Ray;
	class Renderer;
	struct FaceSample;
	class Sampler;
	class PR_LIB RenderEntity : public Entity
	{
	public:
		RenderEntity(const std::string& name);
		virtual ~RenderEntity();

		bool isRenderable() const override;

		virtual bool isLight() const = 0;
		virtual float surfaceArea(Material* m) const = 0;

		virtual bool isCollidable() const;
		virtual float collisionCost() const;

		virtual BoundingBox localBoundingBox() const = 0;
		inline BoundingBox worldBoundingBox() const;

		// In world coords
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const = 0;

		// In world coords
		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const = 0;

		// Entity
		virtual void onFreeze() override;

	private:
		BoundingBox calcWorldBoundingBox() const;

		BoundingBox mWorldBoundingBox_Cache;
	};
}

#include "RenderEntity.inl"