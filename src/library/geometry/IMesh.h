#pragma once

#include "BoundingBox.h"

namespace PR
{
	class FacePoint;
	class Ray;
	class Random;
	class Sampler;
	class Material;
	class PR_LIB_INLINE IMesh
	{
	public:
		virtual bool isLight() const = 0;
		virtual BoundingBox boundingBox() const = 0;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t) = 0;
		virtual FacePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const = 0;
		virtual void replaceMaterial(Material* mat) = 0;
	};
}