#include "BiDirectIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "material/Material.h"

#include "sampler/MultiJitteredSampler.h"
#include "math/Projection.h"
#include "math/MSI.h"

#include "photon/PhotonMap.h"

namespace PR
{
	BiDirectIntegrator::BiDirectIntegrator() :
		Integrator(), mThreadData(nullptr), mThreadCount(0)
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
				delete[] mThreadData[i].LightPDF;
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
				delete[] mThreadData[i].LightPDF;
				//delete[] mThreadData[i].LightEntities;
				delete[] mThreadData[i].LightMaxDepth;
			}
			delete[] mThreadData;

			mThreadData = nullptr;
		}

		if (renderer->lights().empty())
			return;

		mThreadCount = renderer->threads();
		mThreadData = new ThreadData[renderer->threads()];
		size_t maxlightsamples = renderer->settings().maxRayDepth() *
			renderer->lights().size() * renderer->settings().maxLightSamples();

		if (!mThreadData || renderer->settings().maxLightSamples() == 0)
			return;

		for (uint32 i = 0; i < mThreadCount; ++i)
		{
			mThreadData[i].LightPos = new float[maxlightsamples*3];
			mThreadData[i].LightFlux = new Spectrum[maxlightsamples];
			mThreadData[i].LightPDF = new float[maxlightsamples];
			//mThreadData[i].LightEntities = new RenderEntity*[mMaxLightSampleCount];
			mThreadData[i].LightMaxDepth = new uint32[renderer->lights().size() * renderer->settings().maxLightSamples()];
		}
	}
	
	constexpr float LightEpsilon = 0.00001f;
	Spectrum BiDirectIntegrator::apply(const Ray& in, RenderContext* context)
	{
		if (context->renderer()->settings().maxLightSamples() == 0)
			return Spectrum();

		Renderer* renderer = context->renderer();
		if (mThreadData && !renderer->lights().empty())
		{
			ThreadData& data = mThreadData[context->threadNumber()];

			const uint32 maxDepth = renderer->settings().maxRayDepth();
			const uint32 maxDiffBounces = renderer->settings().maxDiffuseBounces();

			Ray current = in;
			uint32 lightNr = 0;
			for (RenderEntity* light : renderer->lights())
			{
				MultiJitteredSampler sampler(context->random(), renderer->settings().maxLightSamples());
				for (uint32 i = 0; i < renderer->settings().maxLightSamples(); ++i)
				{
					float* lightPos = &data.LightPos[lightNr * maxDepth * 3];
					Spectrum* lightFlux = &data.LightFlux[lightNr * maxDepth];
					float* lightPDF = &data.LightPDF[lightNr * maxDepth];

					FacePoint lightSample = light->getRandomFacePoint(sampler, i);
					const PM::vec3 lightDir =
						Projection::align(lightSample.normal(),
							Projection::cos_hemi(context->random().getFloat(), context->random().getFloat()));

					current.setStartPosition(lightSample.vertex());
					current.setDirection(lightDir);
					current.setDepth(1);

					// Initiate with power
					Spectrum flux = lightSample.material()->applyEmission(lightSample, lightSample.normal());

					uint32 lightDepth = 0;// Counts diff bounces
					PM::pm_Store3D(current.startPosition(), &lightPos[lightDepth * 3]);
					lightFlux[lightDepth] = flux;
					lightPDF[lightDepth] = 1;

					for (uint32 k = 1; k < maxDepth && lightDepth <= maxDiffBounces; ++k)
					{
						FacePoint collision;
						RenderEntity* entity = context->shoot(current, collision);
						if (entity && collision.material() && collision.material()->canBeShaded())
						{
							const float NdotL = std::abs(PM::pm_Dot3D(collision.normal(), current.direction()));
							if (NdotL < PM_EPSILON)
								break;

							PR_DEBUG_ASSERT(NdotL <= 1);

							Ray out;
							float pdf;
							PM::vec3 s = PM::pm_Set(context->random().getFloat(),
								context->random().getFloat(),
								context->random().getFloat());
							out.setDirection(collision.material()->sample(collision, s, current.direction(), pdf));
							out.setDepth(in.depth() + 1);
							out.setStartPosition(collision.vertex());

							if (pdf <= PM_EPSILON)
							{
								flux *= 0;
								break;
							}
							else
							{
								flux *= collision.material()->apply(collision, out.direction(), current.direction()) *
									(NdotL / (std::isinf(pdf) ? 1 : pdf));
								current = out;

								if (!std::isinf(pdf))
								{
									lightDepth++;

									lightFlux[lightDepth] = flux;
									lightPDF[lightDepth] = pdf;
									PM::pm_Store3D(current.startPosition(), &lightPos[lightDepth * 3]);
								}
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

		return applyRay(in, context, 0);
	}

	Spectrum BiDirectIntegrator::applyRay(const Ray& in, RenderContext* context, uint32 diffBounces)
	{
		const uint32 maxLights = context->renderer()->settings().maxLightSamples()*(uint32)context->renderer()->lights().size();
		const uint32 maxDepth = context->renderer()->settings().maxRayDepth();

		FacePoint point;
		Spectrum applied;
		Spectrum full_weight;
		float full_pdf = 0;

		RenderEntity* entity = context->shootWithApply(applied, in, point);
		if (entity && point.material() && point.material()->canBeShaded())
		{
			MultiJitteredSampler sampler(context->random(), context->renderer()->settings().maxLightSamples());
			for (uint32 i = 0; i < context->renderer()->settings().maxLightSamples(); ++i)
			{
				float pdf;
				Spectrum weight;
				PM::vec3 rnd = sampler.generate3D(i);
				PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf);
				const float NdotL = std::abs(PM::pm_Dot3D(dir, point.normal()));

				if (NdotL > PM_EPSILON &&
					(std::isinf(pdf) || diffBounces <= context->renderer()->settings().maxDiffuseBounces()))
				{
					Spectrum applied = applyRay(Ray(point.vertex(), dir, in.depth() + 1),
						context, !std::isinf(pdf) ? diffBounces + 1 : diffBounces);

					weight = point.material()->apply(point, in.direction(), dir) * applied * NdotL;
				}

				MSI::balance(full_weight, full_pdf, weight, pdf);
			}

			if (!std::isinf(full_pdf) && maxLights > 0 && mThreadData)
			{
				const ThreadData& data = mThreadData[context->threadNumber()];

				for (uint32 j = 0; j < maxLights; ++j)// Each light
				{
					for (uint32 s = 0; s < data.LightMaxDepth[j]; ++s)
					{
						const float* lightPosP = &data.LightPos[(j * maxDepth + s)*3];
						PM::vec3 lightPos = PM::pm_Set(lightPosP[0], lightPosP[1], lightPosP[2], 1);
						const Spectrum& lightFlux = data.LightFlux[j * maxDepth + s];

						Spectrum weight;
						Ray shootRay(lightPos,
							PM::pm_Normalize3D(PM::pm_Subtract(point.vertex(), lightPos)));

						float pdf = point.material()->pdf(point, in.direction(), shootRay.direction());

						FacePoint tmpCollision;
						if (context->shoot(shootRay, tmpCollision) == entity &&
							PM::pm_MagnitudeSqr3D(PM::pm_Subtract(point.vertex(), tmpCollision.vertex())) <= LightEpsilon)
						{
							const float NdotL = std::abs(PM::pm_Dot3D(tmpCollision.normal(), shootRay.direction()));
							if (NdotL > PM_EPSILON)
							{
								weight = point.material()->apply(point,
									in.direction(), shootRay.direction()) * lightFlux * NdotL;
								PR_DEBUG_ASSERT(!weight.hasNaN());
							}
						}

						MSI::balance(full_weight, full_pdf, weight, pdf);
					}
				}
			}
		}

		return applied + full_weight;
	}
}