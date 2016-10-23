#pragma once

#include "OnePassIntegrator.h"

namespace PR
{
	class PR_LIB DebugIntegrator : public OnePassIntegrator
	{
	public:
		DebugIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context, uint32 pass, ShaderClosure& sc) override;
	};
}