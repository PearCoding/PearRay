#include "BiDirectIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "shader/FaceSample.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "material/Material.h"

#include "sampler/MultiJitteredSampler.h"
#include "math/Projection.h"
#include "math/MSI.h"

namespace PR
{
	BiDirectIntegrator::BiDirectIntegrator() :
		OnePassIntegrator(), mThreadData(nullptr), mThreadCount(0)
	{

	}

	BiDirectIntegrator::~BiDirectIntegrator()
	{
		if (mThreadData)
		{
			for (uint32 i = 0; i < mThreadCount; ++i)
			{
				delete[] mThreadData[i].LightPos;
				delete[] mThreadData[i].LightFlux;
				//delete[] mThreadData[i].LightPDF;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;
		}
	}

	void BiDirectIntegrator::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);

		if (mThreadData)
		{
			for (uint32 i = 0; i < mThreadCount; ++i)
			{
				delete[] mThreadData[i].LightPos;
				delete[] mThreadData[i].LightFlux;
				//delete[] mThreadData[i].LightPDF;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;

			mThreadData = nullptr;
		}

		if (renderer->lights().empty() || renderer->settings().maxLightSamples() == 0)
			return;

		mThreadCount = renderer->threads();
		mThreadData = new ThreadData[renderer->threads()];
		size_t maxlightsamples = renderer->settings().maxRayDepth() *
			renderer->lights().size() * renderer->settings().maxLightSamples();

		if (!mThreadData)
			return;

		for (uint32 i = 0; i < mThreadCount; ++i)
		{
			mThreadData[i].LightPos = new float[maxlightsamples*3];
			mThreadData[i].LightFlux = new Spectrum[maxlightsamples];
			//mThreadData[i].LightPDF = new float[maxlightsamples];
			//mThreadData[i].LightEntities = new RenderEntity*[mMaxLightSampleCount];
			mThreadData[i].LightMaxDepth = new uint32[renderer->lights().size() * renderer->settings().maxLightSamples()];
		}
	}
	
	constexpr float LightEpsilon = 0.00001f;
	Spectrum BiDirectIntegrator::apply(const Ray& in, RenderContext* context, uint32 pass, ShaderClosure& sc)
	{
		if (context->renderer()->settings().maxLightSamples() == 0)
			return Spectrum();

		ShaderClosure other_sc;

		Renderer* renderer = context->renderer();
		MultiJitteredSampler sampler(context->random(), renderer->settings().maxLightSamples());
		if (mThreadData && !renderer->lights().empty())
		{
			ThreadData& data = mThreadData[context->threadNumber()];

			const uint32 maxDepth = renderer->settings().maxRayDepth();
			const uint32 maxDiffBounces = renderer->settings().maxDiffuseBounces();

			uint32 lightNr = 0;
			for (RenderEntity* light : renderer->lights())
			{
				for (uint32 i = 0; i < renderer->settings().maxLightSamples(); ++i)
				{
					float* lightPos = &data.LightPos[lightNr * maxDepth * 3];
					Spectrum* lightFlux = &data.LightFlux[lightNr * maxDepth];

					float pdf;// Area to Solid Angle?
					other_sc = light->getRandomFacePoint(sampler, i, pdf);

					// Initiate with power
					if(!other_sc.Material->emission())
						continue;

					Spectrum flux = other_sc.Material->emission()->eval(other_sc);

					uint32 lightDepth = 0;// Counts diff bounces
					PM::pm_Store3D(other_sc.P, &lightPos[lightDepth * 3]);
					lightFlux[lightDepth] = flux;
					//lightPDF[lightDepth] = pdf;

					Ray current = in;
					current.setFlags(current.flags() | RF_FromLight);

					for (uint32 k = 1; k < maxDepth && lightDepth <= maxDiffBounces; ++k)
					{
						PM::vec3 s = PM::pm_Set(context->random().getFloat(),
								context->random().getFloat(),
								context->random().getFloat());
						PM::vec3 dir = other_sc.Material->sample(other_sc, s, pdf);

						if (pdf <= PM_EPSILON)
							break;
						
						current = current.next(other_sc.P, dir);

						RenderEntity* entity = context->shoot(current, other_sc);
						if (entity &&
							other_sc.Material && other_sc.Material->canBeShaded())
						{
							const float NdotL = std::abs(PM::pm_Dot3D(other_sc.N, current.direction()));
							if (NdotL <= PM_EPSILON)
								break;

							flux *=	other_sc.Material->eval(other_sc, PM::pm_Negate(current.direction()), NdotL) * NdotL;

							if (!std::isinf(pdf))
							{
								lightDepth++;
								lightFlux[lightDepth] = flux / pdf;
								PM::pm_Store3D(other_sc.P, &lightPos[lightDepth * 3]);
							}
						}
						else
						{
							break;
						}
					}

					data.LightMaxDepth[lightNr] = lightDepth;
					lightNr++;
				}
			}
		}

		return applyRay(in, context, 0, sc);
	}

	Spectrum BiDirectIntegrator::applyRay(const Ray& in, RenderContext* context, uint32 diffBounces, ShaderClosure& sc)
	{
		const uint32 maxLights = context->renderer()->settings().maxLightSamples()*(uint32)context->renderer()->lights().size();
		const uint32 maxDepth = context->renderer()->settings().maxRayDepth();

		if(in.depth() >= maxDepth)
			return Spectrum();

		Spectrum applied;
		Spectrum full_weight;
		float full_pdf = 0;

		// Temporary
		ShaderClosure other_sc;
		Spectrum other_weight;

		RenderEntity* entity = context->shootWithEmission(applied, in, sc);
		if (!entity || !sc.Material || !sc.Material->canBeShaded())
			return applied;

		MultiJitteredSampler sampler(context->random(), context->renderer()->settings().maxLightSamples());
		for (uint32 i = 0;
			 i < context->renderer()->settings().maxLightSamples() &&
			 	!std::isinf(full_pdf);
			 ++i)
		{
			float other_pdf = 0;
			Spectrum other_weight;

			PM::vec3 rnd = sampler.generate3D(i);
			for(uint32 path = 0; path < sc.Material->samplePathCount() && !std::isinf(other_pdf); ++path)
			{
				Spectrum weight;
				float pdf;
				float path_weight;
				PM::vec3 dir = sc.Material->samplePath(sc, rnd, pdf, path_weight, path);

				if(pdf > PM_EPSILON && path_weight > PM_EPSILON)
				{
					const float NdotL = std::abs(PM::pm_Dot3D(dir, sc.N));

					if (NdotL > PM_EPSILON &&
						(std::isinf(pdf) || diffBounces < context->renderer()->settings().maxDiffuseBounces()))
					{
						weight = applyRay(in.next(sc.P, dir),
							context, !std::isinf(pdf) ? diffBounces + 1 : diffBounces,
							other_sc);

						weight *= sc.Material->eval(sc, dir, NdotL) * NdotL;
					}
				}

				other_pdf += path_weight*pdf;
				other_weight += path_weight*weight;
			}

			MSI::power(full_weight, full_pdf, other_weight, other_pdf);
		}

		if (!std::isinf(full_pdf))
		{
			if(mThreadData)
			{
				const ThreadData& data = mThreadData[context->threadNumber()];

				for (uint32 j = 0; j < maxLights; ++j)// Each light
				{
					for (uint32 s = 0; s < data.LightMaxDepth[j]; ++s)
					{
						const float* lightPosP = &data.LightPos[(j * maxDepth + s)*3];
						PM::vec3 lightPos = PM::pm_Set(lightPosP[0], lightPosP[1], lightPosP[2], 1);
						const Spectrum& lightFlux = data.LightFlux[j * maxDepth + s];
						const auto LP = PM::pm_Subtract(sc.P, lightPos);

						Ray current = Ray::safe(in.pixel(),
							lightPos,
							PM::pm_Normalize3D(LP),
							in.depth() + 1,
							in.time(),
							in.flags() | RF_FromLight,
							in.maxDepth());

						const PM::vec3 L = PM::pm_Negate(current.direction());

						const float NdotL = PM::pm_Max(0.0f, PM::pm_Dot3D(sc.N, L));
						if(NdotL <= PM_EPSILON)
							continue;

						const float pdf = sc.Material->pdf(sc, L, NdotL);

						if (context->shoot(current, other_sc) == entity &&
								PM::pm_MagnitudeSqr3D(PM::pm_Subtract(sc.P, other_sc.P)) <= LightEpsilon)
							other_weight = lightFlux * sc.Material->eval(sc, L, NdotL) * NdotL;
						else
							other_weight.clear();

						MSI::power(full_weight, full_pdf, other_weight, pdf);
					}
				}
			}

			float inf_pdf;
			other_weight = handleInfiniteLights(in, sc, context, inf_pdf);
			MSI::power(full_weight, full_pdf, other_weight, inf_pdf);
		}

		return applied + full_weight;
	}
}