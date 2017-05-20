#include "DirectIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "entity/RenderEntity.h"
#include "sampler/RandomSampler.h"
#include "material/Material.h"
#include "math/MSI.h"

namespace PR
{
	DirectIntegrator::DirectIntegrator(RenderContext* renderer) : OnePassIntegrator(renderer)
	{
	}

	void DirectIntegrator::init()
	{
	}

	Spectrum DirectIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
	{
		Spectrum applied;
		RenderEntity* entity = context->shootWithEmission(applied, in, sc);

		if (!entity || !sc.Material)
			return applied;

		return applied + applyRay(in, sc, context, 0);
	}

	Spectrum DirectIntegrator::applyRay(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, uint32 diffbounces)
	{
		if (!sc.Material->canBeShaded())
			return Spectrum();

		float full_pdf = 0;
		Spectrum full_weight;

		// Used temporary
		ShaderClosure other_sc;
		float path_weight;
		Spectrum weight;

		// Hemisphere sampling
		RandomSampler hemiSampler(context->random());
		for (uint32 i = 0;
			 i < renderer()->settings().maxLightSamples() && !std::isinf(full_pdf);
			 ++i)
		{
			Spectrum other_weight;
			float other_pdf = 0;

			const uint32 path_count = sc.Material->samplePathCount();
			PR_ASSERT(path_count > 0, "path_count should be always higher than 0.");

			Eigen::Vector3f rnd = hemiSampler.generate3D(i);
			for(uint32 path = 0; path < path_count; ++path)
			{
				float pdf;

				Eigen::Vector3f dir = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);
				const float NdotL = std::abs(dir.dot(sc.N));

				if (pdf <= PR_EPSILON || NdotL <= PR_EPSILON ||
					!(std::isinf(pdf) || diffbounces < renderer()->settings().maxDiffuseBounces()))
					continue;

				Ray ray = in.next(sc.P, dir);

				RenderEntity* entity = context->shootWithEmission(weight, ray, other_sc);
				if (entity && other_sc.Material)
					weight += applyRay(ray, other_sc, context,
						!std::isinf(pdf) ? diffbounces + 1 : diffbounces);

				weight *= sc.Material->eval(sc, dir, NdotL) * NdotL;
				other_weight += path_weight*weight;
				other_pdf += pdf;
			}
			MSI::balance(full_weight, full_pdf, other_weight, other_pdf / path_count);
		}

		if (!std::isinf(full_pdf))
		{
			Spectrum other_weight;
			// Area sampling!
			for (RenderEntity* light : renderer()->lights())
			{
				RandomSampler sampler(context->random());
				for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i)
				{
					float pdf;
					FaceSample p = light->getRandomFacePoint(sampler, i, pdf);

					const Eigen::Vector3f PS = p.P-sc.P;
					const Eigen::Vector3f L = PS.normalized();
					const float NdotL = std::max(0.0f, L.dot(sc.N));// No back light detection

					pdf = MSI::toSolidAngle(pdf, PS.squaredNorm(), NdotL) +
						sc.Material->pdf(sc, L, NdotL);

					if (pdf <= PR_EPSILON)
						continue;

					if (NdotL > PR_EPSILON)
					{
						Ray ray = in.next(sc.P, L);
						ray.setFlags(ray.flags() | RF_Light);

						if (context->shootWithEmission(other_weight, ray, other_sc) == light)// Full light!!
							other_weight *= sc.Material->eval(sc, L, NdotL) * NdotL;
						else
							other_weight.clear();
					}
					else
						other_weight.clear();

					MSI::balance(full_weight, full_pdf,
						other_weight, std::isinf(pdf) ? 1 : pdf);
				}
			}

			float inf_pdf;
			other_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::balance(full_weight, full_pdf, other_weight, inf_pdf);
		}

		return full_weight;
	}
}
