#pragma once

#include "Config.h"

namespace PR
{
	class RenderEntity;
	class FacePoint;
	class Ray;
	class Renderer;
	class PR_LIB_INLINE Material
	{
	public:
		virtual void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer) = 0;
		virtual bool isLight() const = 0;

		inline virtual bool shouldIgnore_Simple(const Ray& in, RenderEntity* entity)
		{
			return false;
		}

		inline virtual bool shouldIgnore_Complex(const Ray& in, RenderEntity* entity, const FacePoint& point)
		{
			return false;
		}
	};
}