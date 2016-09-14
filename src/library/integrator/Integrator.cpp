#include "Integrator.h"

#include "spectral/Spectrum.h"
#include "light/IInfiniteLight.h"

#include "sampler/RandomSampler.h"

#include "renderer/RenderContext.h"

#include "scene/Scene.h"

#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "ray/Ray.h"

#include "math/MSI.h"

namespace PR
{
	Spectrum Integrator::handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderContext* context, float& full_pdf)
	{
		Spectrum full_weight;
		full_pdf = 0;

		RandomSampler hemiSampler(context->random());
		for(IInfiniteLight* e : context->renderer()->scene()->infiniteLights())
		{
			Spectrum semi_weight;
			float semi_pdf = 0;

			for(uint32 i = 0;
				i < context->renderer()->settings().maxLightSamples() && !std::isinf(semi_pdf);
				++i)
			{
				float pdf;
				PM::vec3 rnd = hemiSampler.generate3D(i);
				PM::vec3 dir = e->sample(sc, rnd, pdf);

				if(pdf > PM_EPSILON)
				{
					Spectrum weight;
					const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(dir, sc.N));

					if (NdotL > PM_EPSILON)
					{
						Ray ray = in.next(sc.P, dir);
						if (!context->shootForDetection(ray))
							weight = sc.Material->apply(sc, dir) * e->apply(ray.direction()) * NdotL;
					} 
				
					MSI::power(semi_weight, semi_pdf, weight, pdf);
				}
			}
			
			MSI::balance(full_weight, full_pdf, semi_weight, std::isinf(semi_pdf) ? 1 : semi_pdf);
		}

		return full_weight;
	}
}