#pragma once

#include "OnePassIntegrator.h"

namespace PR
{
	class PR_LIB DirectIntegrator : public OnePassIntegrator
	{
	public:
		DirectIntegrator(Renderer* renderer);

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context, uint32 pass) override;

	private:
		Spectrum applyRay(const Ray& in, const SamplePoint& point, RenderContext* context);
	};
}