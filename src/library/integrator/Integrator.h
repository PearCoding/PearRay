#pragma once

#include "Config.h"

namespace PR
{
	class RenderEntity;
	class FacePoint;
	class Ray;
	class Renderer;
	class RenderContext;
	class Spectrum;
	class Integrator
	{
	public:
		virtual void init(Renderer* renderer) = 0;

		virtual Spectrum apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) = 0;
	};
}