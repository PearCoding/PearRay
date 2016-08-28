#pragma once

#include "Entity.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	class Material;
	class Random;
	class Ray;
	class Renderer;
	struct SamplePoint;
	class Sampler;
	class PR_LIB RenderEntity : public Entity
	{
	public:
		RenderEntity(const std::string& name, Entity* parent = nullptr);
		virtual ~RenderEntity();

		bool isRenderable() const override;

		virtual bool isLight() const = 0;

		virtual bool isCollidable() const;
		virtual float collisionCost() const;

		virtual BoundingBox localBoundingBox() const;
		BoundingBox worldBoundingBox() const;

		// In world coords
		virtual bool checkCollision(const Ray& ray, SamplePoint& collisionPoint, float& t);

		// In world coords
		virtual SamplePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const = 0;

		// Entity
		virtual void onPreRender() override;

	private:
		BoundingBox calcWorldBoundingBox() const;

		bool mFrozen;
		BoundingBox mWorldBoundingBox_Cache;
	};
}