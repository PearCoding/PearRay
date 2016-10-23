#pragma once

#include "OnePassIntegrator.h"

namespace PR
{
	struct ShaderClosure;
	class PR_LIB DirectIntegrator : public OnePassIntegrator
	{
	public:
		DirectIntegrator(Renderer* renderer);

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context, uint32 pass, ShaderClosure& sc) override;

	private:
		Spectrum applyRay(const Ray& in, const ShaderClosure& sc, RenderContext* context, uint32 diffbounces);
	};
}