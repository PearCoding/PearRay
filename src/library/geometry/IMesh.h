#pragma once

#include "BoundingBox.h"

namespace PR
{
	class FacePoint;
	class Ray;
	class Random;
	class Sampler;
	class PR_LIB_INLINE IMesh
	{
	public:
		virtual BoundingBox boundingBox() const = 0;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) = 0;
		virtual FacePoint getRandomFacePoint(Sampler& sampler, Random& random) const = 0;
	};
}