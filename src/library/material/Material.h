#pragma once

#include "Config.h"

namespace PR
{
	class Entity;
	class FacePoint;
	class Ray;
	class Renderer;
	class PR_LIB_INLINE Material
	{
	public:
		virtual void apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer) = 0;
	};
}