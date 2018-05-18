#include "DirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "math/MSI.h"
#include "math/Projection.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"
#include "sampler/RandomSampler.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

/* Registry Entries: */
static const char* RE_LIGHT_SAMPLES = "direct/light/sample_count";
static const char* RE_DIFFUSE_DEPTH = "direct/diffuse/max_depth";

namespace PR {
struct DI_ThreadData {
	Spectrum LiWeight;
	Spectrum IntegralWeight;
	Spectrum Evaluation;
	std::vector<Spectrum> HemiWeights;
	std::vector<Spectrum> PathWeights;

	explicit DI_ThreadData(RenderContext* context)
		: LiWeight(context->spectrumDescriptor())
		, IntegralWeight(context->spectrumDescriptor())
		, Evaluation(context->spectrumDescriptor())
	{
		for (uint32 i = 0; i < context->settings().maxRayDepth(); ++i) {
			HemiWeights.emplace_back(context->spectrumDescriptor());
			PathWeights.emplace_back(context->spectrumDescriptor());
		}
	}
};

DirectIntegrator::DirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

DirectIntegrator::~DirectIntegrator()
{
}

void DirectIntegrator::init()
{
	OnePassIntegrator::init();

	mThreadData.clear();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());

	mMaxLightSamples = std::max<uint32>(1,
										renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				   RE_LIGHT_SAMPLES,
																				   1));
	mMaxDiffBounces = renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																 RE_DIFFUSE_DEPTH,
																 1);
	mLightCount = renderer()->lights().size();
}

constexpr float LightEpsilon = 0.00001f;
void DirectIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	applyRay(spec, in, session, 0, sc);
}

void DirectIntegrator::applyRay(Spectrum& spec, const Ray& in,
								const RenderSession& session,
								uint32 diffbounces, ShaderClosure& sc)
{
	const RenderEntity* entity = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded())
		return;

	const bool specular = addIndirectContribution(spec, in, session, diffbounces, sc);

	if (!specular) {
		addDirectContribution(spec, in, session, sc);
		addInfiniteLightContribution(spec, in, sc, session, mMaxLightSamples);
	}
}

bool DirectIntegrator::addIndirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc)
{
	DI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth		  = in.depth();

	const Eigen::Vector3f rnd_hemi = session.tile()->random().get3D();
	const bool specular			   = sampleHemisphere(threadData.HemiWeights[depth],
										   rnd_hemi, in, session, diffbounces, sc);
	spec += threadData.HemiWeights[depth];

	return specular;
}

void DirectIntegrator::addDirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	DI_ThreadData& threadData = mThreadData[session.thread()];
	const float f			  = mLightCount / (float)mMaxLightSamples;

	threadData.IntegralWeight.clear();
	for (uint32 i = 0;
		 i < mMaxLightSamples;
		 ++i) {
		const Eigen::Vector3f rnd_area = session.tile()->random().get3D();
		const float rnd2			   = session.tile()->random().getFloat();

		const uint32 lightNr = Projection::map<uint32>(rnd2, 0, mLightCount - 1);
		RenderEntity* light  = renderer()->lights().at(lightNr);
		sampleLight(threadData.LiWeight, light, rnd_area, in, session, sc);

		threadData.IntegralWeight += threadData.LiWeight;
	}

	spec += threadData.IntegralWeight * f;
}

bool DirectIntegrator::sampleHemisphere(Spectrum& spec, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc)
{
	spec.clear();

	DI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth		  = in.depth();
	const uint32 path_count   = sc.Material->samplePathCount();

	bool specular = false;
	for (uint32 path = 0; path < path_count; ++path) {
		const MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);

		float NdotL = 0;
		if (ms.isReflection())
			NdotL = std::max(0.0f, ms.L.dot(sc.N));
		else
			NdotL = std::max(0.0f, -ms.L.dot(sc.N));

		specular = specular || ms.isSpecular();

		const uint32 nextDiffBounces = !ms.isSpecular() ? diffbounces + 1 : diffbounces;
		if (ms.PDF_S <= PR_EPSILON || NdotL <= PR_EPSILON
			|| ms.PathWeight <= PR_EPSILON || nextDiffBounces > mMaxDiffBounces) {
			continue;
		}

		ShaderClosure other_sc;
		applyRay(threadData.PathWeights[depth], in.next(sc.P, ms.L), session,
				 nextDiffBounces, other_sc);

		sc.Material->eval(threadData.Evaluation, sc, ms.L, NdotL, session);
		threadData.PathWeights[depth] *= threadData.Evaluation
										 * (ms.PathWeight * NdotL / ms.safePDF_S());

		spec += threadData.PathWeights[depth];
	}

	return specular;
}

void DirectIntegrator::sampleLight(Spectrum& spec, RenderEntity* light, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc)
{
	DI_ThreadData& threadData = mThreadData[session.thread()];

	const RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd);

	const Eigen::Vector3f PS = fps.Point.P - sc.P;
	const Eigen::Vector3f L  = PS.normalized();
	const float NdotL		 = std::max(0.0f, L.dot(sc.N)); // No back light detection

	float pdfA = fps.PDF_A;

	if (pdfA <= PR_EPSILON || NdotL <= PR_EPSILON) {
		spec.clear();
		return;
	}

	Ray ray = in.next(sc.P, L);
	ray.setFlags(ray.flags() | RF_Light);

	ShaderClosure other_sc;
	if (renderer()->shootWithEmission(spec, ray, other_sc, session) == light
		&& (fps.Point.P - other_sc.P).squaredNorm() <= LightEpsilon) {
		if (other_sc.Flags & SCF_Inside) // Wrong side (Back side)
			return;

		const float geom_term = NdotL * std::abs(other_sc.NdotV) /* other_sc.Depth2*/;

		sc.Material->eval(threadData.Evaluation, sc, L, NdotL, session);
		spec *= threadData.Evaluation * (geom_term / pdfA);
	}
}
} // namespace PR
