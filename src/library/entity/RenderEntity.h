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
		virtual BoundingBox worldBoundingBox() const;

		// In world coords
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t);

		// In world coords
		virtual FacePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const = 0;
	};
}