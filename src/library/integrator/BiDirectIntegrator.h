#pragma once

#include "Integrator.h"

namespace PR
{
	class PR_LIB BiDirectIntegrator : public Integrator
	{
	public:
		BiDirectIntegrator();
		~BiDirectIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context) override;

	private:
		Spectrum applyRay(const Ray& in, RenderContext* context, uint32 diffBounces);

		struct ThreadData
		{
			float* LightPos;// For every light a path (LightCount * MaxLightSamples * MaxPathCount * 3)
			Spectrum* LightFlux;
			uint32* LightMaxDepth;
			float* LightPDF;
		};
		ThreadData* mThreadData;
		uint32 mThreadCount;
	};
}