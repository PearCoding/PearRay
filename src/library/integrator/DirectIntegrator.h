#pragma once

#include "OnePassIntegrator.h"

namespace PR
{
	struct ShaderClosure;
	class PR_LIB DirectIntegrator : public OnePassIntegrator
	{
	public:
		DirectIntegrator(RenderContext* renderer);

		void init() override;
		Spectrum apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc) override;

	private:
		Spectrum applyRay(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, uint32 diffbounces);
	};
}
