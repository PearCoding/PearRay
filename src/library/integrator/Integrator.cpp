#include "Integrator.h"

#include "light/IInfiniteLight.h"
#include "spectral/Spectrum.h"

#include "sampler/RandomSampler.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"

#include "scene/Scene.h"

#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "ray/Ray.h"

#include "math/MSI.h"

// Implementations
#include "integrator/BiDirectIntegrator.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/PPMIntegrator.h"

namespace PR {
struct I_ThreadData {
	Spectrum SemiWeight;
	Spectrum Weight;
	Spectrum Evaluation;

	explicit I_ThreadData(RenderContext* context)
		: SemiWeight(context->spectrumDescriptor())
		, Weight(context->spectrumDescriptor())
		, Evaluation(context->spectrumDescriptor())
	{
	}
};

Integrator::Integrator(RenderContext* renderer)
	: mRenderer(renderer)
{
}

Integrator::~Integrator()
{
}

void Integrator::init()
{
	mThreadData.clear();
	for (uint32 i = 0; i < mRenderer->threads(); ++i)
		mThreadData.emplace_back(mRenderer);
}

void Integrator::handleInfiniteLights(Spectrum& spec, const Ray& in, const ShaderClosure& sc, const RenderSession& session, float& full_pdf)
{
	I_ThreadData& threadData = mThreadData[session.thread()];
	full_pdf				 = 0;

	if (renderer()->scene()->infiniteLights().empty())
		return;

	const float lightSampleWeight   = 1.0f / renderer()->settings().maxLightSamples();
	const float inflightCountWeight = 1.0f / renderer()->scene()->infiniteLights().size();

	RandomSampler sampler(session.tile()->random());
	for (auto e : renderer()->scene()->infiniteLights()) {
		float semi_pdf = 0;

		for (uint32 i = 0;
			 i < renderer()->settings().maxLightSamples() && !std::isinf(semi_pdf);
			 ++i) {
			IInfiniteLight::LightSample ls = e->sample(sc, sampler.generate3D(i), session);

			if (ls.PDF_S <= PR_EPSILON)
				continue;

			const float NdotL = std::abs(ls.L.dot(sc.N));

			if (NdotL <= PR_EPSILON)
				continue;

			Ray ray = in.next(sc.P, ls.L);
			ray.setFlags(ray.flags() | RF_Light);

			if (!mRenderer->shootForDetection(ray, session)) {
				sc.Material->eval(threadData.Weight, sc, ls.L, NdotL, session);
				e->apply(threadData.Evaluation, ls.L, session);
				threadData.Weight *= threadData.Evaluation * NdotL;
			}

			MSI::balance(threadData.SemiWeight, semi_pdf, threadData.Weight, lightSampleWeight * ls.PDF_S);
		}

		MSI::balance(spec, full_pdf, threadData.SemiWeight,
					 inflightCountWeight * (std::isinf(semi_pdf) ? 1 : semi_pdf));
	}
}

void Integrator::handleSpecularPath(Spectrum& spec, const Ray& in, const ShaderClosure& sc, const RenderSession& session, RenderEntity*& lastEntity)
{
	I_ThreadData& threadData = mThreadData[session.thread()];
	ShaderClosure other_sc;
	Ray ray = in;

	lastEntity = mRenderer->shoot(ray, other_sc, session);

	if (lastEntity && other_sc.Material) {
		float NdotL = std::max(0.0f, ray.direction().dot(other_sc.N));
		other_sc.Material->eval(spec, other_sc, ray.direction(), NdotL, session);
		spec *= NdotL;

		for (uint32 depth = in.depth();
			 depth < renderer()->settings().maxRayDepth();
			 ++depth) {
			MaterialSample ms = other_sc.Material->sample(other_sc,
														  session.tile()->random().get3D(),
														  session);

			if (!std::isinf(ms.PDF_S))
				break;

			float NdotL = std::max(0.0f, ray.direction().dot(other_sc.N));
			if (NdotL <= PR_EPSILON)
				break;

			ray = ray.next(other_sc.P, ms.L);

			lastEntity = mRenderer->shoot(ray, other_sc, session);
			if (lastEntity && other_sc.Material) {
				other_sc.Material->eval(threadData.Evaluation, other_sc, ms.L, NdotL, session);
				spec *= threadData.Evaluation * NdotL;
			} else {
				break;
			}
		}
	}
}

std::unique_ptr<Integrator> Integrator::create(RenderContext* context, IntegratorMode mode) {
	switch (mode) {
	case IM_Direct:
		return std::make_unique<DirectIntegrator>(context);
	default:
	case IM_BiDirect:
		return std::make_unique<BiDirectIntegrator>(context);
	case IM_PPM:
		return std::make_unique<PPMIntegrator>(context);
	}
}

std::unique_ptr<Integrator> Integrator::createDebug(RenderContext* context) {
	return std::make_unique<DebugIntegrator>(context);
}
} // namespace PR
