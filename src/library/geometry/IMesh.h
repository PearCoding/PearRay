#pragma once

#include "BoundingBox.h"

namespace PR
{
	class Material;
	class Ray;
	class Random;
	struct FaceSample;
	class Sampler;
	class PR_LIB IMesh
	{
	public:
		virtual ~IMesh() {}

		virtual bool isLight() const = 0;
		virtual float surfaceArea(Material* m, const PM::mat& transform) const = 0;

		virtual BoundingBox boundingBox() const = 0;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) = 0;
		virtual float collisionCost() const = 0;
		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const = 0;
		virtual void replaceMaterial(Material* mat) = 0;
	};
}