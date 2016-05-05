#pragma once

#include "Integrator.h"
#include "sampler/StratifiedSampler.h"

namespace PR
{
	class PR_LIB DirectIntegrator : public Integrator
	{
	public:
		DirectIntegrator(Renderer* renderer, uint32 lightSamples);

		void init(Renderer* renderer) override;
		Spectrum apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) override;

	private:
		uint32 mLightSamples;
		StratifiedSampler mLightSampler;
	};
}