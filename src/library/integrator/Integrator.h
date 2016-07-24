#pragma once

#include "Config.h"

namespace PR
{
	class RenderEntity;
	struct SamplePoint;
	class Ray;
	class Renderer;
	class RenderContext;
	class Spectrum;
	class Integrator
	{
	public:
		virtual void init(Renderer* renderer) = 0;

		virtual Spectrum apply(const Ray& in, RenderContext* context) = 0;
	};
}