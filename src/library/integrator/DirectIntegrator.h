#pragma once

#include "Integrator.h"
#include "sampler/StratifiedSampler.h"

namespace PR
{
	class PR_LIB DirectIntegrator : public Integrator
	{
	public:
		DirectIntegrator(Renderer* renderer);

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context) override;

	private:
		Spectrum applyRay(const Ray& in, const FacePoint& point, RenderEntity* entity, RenderContext* context);

		StratifiedSampler mLightSampler;
	};
}