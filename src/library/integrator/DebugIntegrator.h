#pragma once

#include "Integrator.h"

namespace PR
{
	class PR_LIB DebugIntegrator : public Integrator
	{
	public:
		DebugIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context) override;
	};
}