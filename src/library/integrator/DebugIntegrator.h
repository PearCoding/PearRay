#pragma once

#include "OnePassIntegrator.h"

namespace PR
{
	class PR_LIB DebugIntegrator : public OnePassIntegrator
	{
	public:
		DebugIntegrator(RenderContext* renderer);

		void init() override;
		Spectrum apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc) override;
	};
}
