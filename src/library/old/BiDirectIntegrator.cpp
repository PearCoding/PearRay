#include "BiDirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

#include "material/Material.h"

#include "math/MSI.h"
#include "math/Projection.h"
#include "sampler/MultiJitteredSampler.h"

#include <vector>

namespace PR {

static const char* RE_LIGHT_SAMPLES = "bidirect/light/sample_count";
static const char* RE_LIGHT_DEPTH   = "bidirect/light/max_depth";
static const char* RE_DIFFUSE_DEPTH = "bidirect/diffuse/max_depth";

struct BIDI_ThreadData {
	struct EventVertex {
		Spectrum CumulativeWeight; // a_i
		float GeometricFactor;	 // G_i G(x_i-1 <-> x_i)
		float CumulativePDF;
		float PDF_Forward;
		float PDF_Backward;
		RenderEntity* VirtualEntity;
		ShaderClosure SC;
		bool Specular;

		inline EventVertex(const std::shared_ptr<SpectrumDescriptor>& specDesc)
			: CumulativeWeight(specDesc)
		{
		}
	};

	std::vector<EventVertex> LightVertices;
	std::vector<uint32> LightPathLength;

	std::vector<EventVertex> EyeVertices;

	Spectrum IntegralWeight;
	Spectrum CumWeight;
	Spectrum NextWeight;
	Spectrum LiWeight;
	Spectrum EyeEmissiveEnd;

	explicit BIDI_ThreadData(RenderContext* context, uint32 lightPatches, uint32 lightDepth, uint32 eyeDepth)
		: LightPathLength(lightPatches, 0)
		, IntegralWeight(context->spectrumDescriptor())
		, CumWeight(context->spectrumDescriptor())
		, NextWeight(context->spectrumDescriptor())
		, LiWeight(context->spectrumDescriptor())
		, EyeEmissiveEnd(context->spectrumDescriptor())
	{
		for (uint32 i = 0; i < lightPatches * (lightDepth + 1); ++i) {
			LightVertices.emplace_back(context->spectrumDescriptor());
		}

		for (uint32 i = 0; i < eyeDepth + 1; ++i) {
			EyeVertices.emplace_back(context->spectrumDescriptor());
		}
	}
};

BiDirectIntegrator::BiDirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

BiDirectIntegrator::~BiDirectIntegrator()
{
}

void BiDirectIntegrator::init()
{
	OnePassIntegrator::init();

	mThreadData.clear();

	mMaxLightSamples = std::max<uint32>(1,
										renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				   RE_LIGHT_SAMPLES,
																				   1));

	mMaxLightDepth  = std::max<uint32>(1,
									   renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				  RE_LIGHT_DEPTH,
																				  1));
	mMaxDiffBounces = renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																 RE_DIFFUSE_DEPTH,
																 1);

	mMaxCameraDepth  = renderer()->settings().maxRayDepth();
	mMaxLightPatches = renderer()->lights().size() * mMaxLightSamples;

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer(), mMaxLightPatches, mMaxLightDepth, mMaxCameraDepth);
}

constexpr float LightEpsilon = 0.00001f;
constexpr float SpecEpsilon  = 0.0001f;
void BiDirectIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	spec.clear();

	buildAllLightPaths(in, session);

	uint32 eyeLength = buildEyePath(in, session);

	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	uint32 goodPatches			= 0;
	for (uint32 patch = 0; patch < mMaxLightPatches; ++patch) {
		if (threadData.LightPathLength[patch] > 0) {
			combineLightEyePath(in, threadData.IntegralWeight, patch, eyeLength, session);
			spec += threadData.IntegralWeight;
			++goodPatches;
		}
	}

	if (goodPatches > 0) {
		spec /= goodPatches;
	}
}

void BiDirectIntegrator::buildAllLightPaths(const Ray& in, const RenderSession& session)
{
	uint32 lightNr = 0;
	for (RenderEntity* light : renderer()->lights()) {
		for (uint32 i = 0; i < mMaxLightSamples; ++i) {
			buildLightPath(in, light, lightNr, session);
			++lightNr;
		}
	}
}

void BiDirectIntegrator::buildLightPath(const Ray& in, RenderEntity* light, uint32 lightNr, const RenderSession& session)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	auto* lightV				= &threadData.LightVertices[lightNr * mMaxLightDepth];

	// y_0 (First light vertex is special!)
	const Eigen::Vector3f rnd		  = session.tile()->random().get3D();
	RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd);

	// Skip light if needed
	if (!fps.Point.Material->isLight() || fps.PDF_A <= PR_EPSILON) {
		threadData.LightPathLength[lightNr] = 0;
		return;
	}

	threadData.CumWeight.fill(1);
	float cumPDF = 1;

	lightV[0].CumulativeWeight.copyFrom(threadData.CumWeight);
	lightV[0].GeometricFactor = 1;
	lightV[0].CumulativePDF   = cumPDF;
	lightV[0].PDF_Backward	= 0;
	lightV[0].SC			  = fps.Point;
	lightV[0].VirtualEntity		  = light;
	lightV[0].Specular		  = false;

	fps.Point.Material->evalEmission(threadData.CumWeight, fps.Point, session, true);
	threadData.CumWeight /= fps.PDF_A; // a_1

	// Sample light direction
	MaterialSample ms;
	ms.L				  = Projection::tangent_align(fps.Point.Ng, fps.Point.Nx, fps.Point.Ny,
									  Projection::cos_hemi(session.tile()->random().getFloat(), session.tile()->random().getFloat(), ms.PDF_S));
	lightV[0].PDF_Forward = ms.PDF_S;

	float NdotL = std::abs(fps.Point.Ng.dot(ms.L));
	Ray current = Ray(in.weight(), in.pixelIndex(),
					  Ray::safePosition(fps.Point.P, ms.L),
					  ms.L,
					  0,
					  in.time(), in.wavelengthIndex(),
					  in.flags() | RF_Light);

	threadData.NextWeight.fill(NdotL / ms.safePDF_S());
	float nextPDF = ms.safePDF_S();

	uint32 diffBounces = 0;
	uint32 lightDepth  = 1;
	while (lightDepth < mMaxLightDepth + 1 && diffBounces <= mMaxDiffBounces
		   && ms.PDF_S > PR_EPSILON && NdotL > PR_EPSILON) {
		ShaderClosure sc;
		RenderEntity* entity = renderer()->shoot(current, sc, session);
		if (entity && sc.Material && sc.Material->canBeShaded() && !sc.Material->isLight()) {
			// Save next vertex
			lightV[lightDepth].VirtualEntity = entity;
			lightV[lightDepth].CumulativeWeight.copyFrom(threadData.CumWeight);
			lightV[lightDepth].CumulativePDF   = cumPDF;
			lightV[lightDepth].GeometricFactor = std::abs(NdotL * sc.NdotV) / (in.origin() - sc.P).squaredNorm();
			lightV[lightDepth].PDF_Backward	= sc.Material->pdf(sc, -sc.V, -sc.NdotV, session);
			lightV[lightDepth].SC			   = sc;

			ms = sc.Material->sample(
				sc, session.tile()->random().get3D(), session);
			NdotL						   = std::abs(sc.N.dot(ms.L));
			lightV[lightDepth].PDF_Forward = ms.safePDF_S();
			lightV[lightDepth].Specular	= ms.isSpecular();
			++lightDepth;

			threadData.CumWeight *= threadData.NextWeight;
			cumPDF *= nextPDF;

			sc.Material->eval(threadData.NextWeight, sc, ms.L, NdotL, session);
			threadData.NextWeight *= /*NdotL*/ 1 / ms.safePDF_S();
			nextPDF = ms.safePDF_S() * lightV[lightDepth - 1].GeometricFactor;

			if (!ms.isSpecular()) {
				++diffBounces;
			}

			if (threadData.NextWeight.isOnlyZero(SpecEpsilon)) {
				break;
			}

			current = current.next(sc.P, ms.L);
		} else {
			break;
		}
	}

	threadData.LightPathLength[lightNr] = lightDepth;
}

uint32 BiDirectIntegrator::buildEyePath(const Ray& in, const RenderSession& session)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	auto* eyeV					= &threadData.EyeVertices[0];

	threadData.EyeEmissiveEnd.clear();
	threadData.CumWeight.fill(1);
	threadData.NextWeight.fill(1);

	float cumPDF  = 1;
	float nextPDF = 1;

	eyeV[0].CumulativeWeight.copyFrom(threadData.CumWeight);
	eyeV[0].CumulativePDF   = cumPDF;
	eyeV[0].GeometricFactor = 1;
	eyeV[0].PDF_Backward	= 0;
	eyeV[0].VirtualEntity			= nullptr;
	eyeV[0].Specular		= false;

	Ray current = in;

	float NdotL = 1;

	uint32 diffBounces = 0;
	uint32 eyeDepth	= 1;
	while (eyeDepth < mMaxCameraDepth + 1 && diffBounces <= mMaxDiffBounces) {
		ShaderClosure sc;
		RenderEntity* entity = renderer()->shootWithEmission(threadData.EyeEmissiveEnd, current, sc, session);
		if (entity && sc.Material && sc.Material->canBeShaded() && !sc.Material->isLight()) {
			// Save next vertex
			eyeV[eyeDepth].VirtualEntity = entity;
			eyeV[eyeDepth].CumulativeWeight.copyFrom(threadData.CumWeight);
			eyeV[eyeDepth].CumulativePDF   = cumPDF;
			eyeV[eyeDepth].GeometricFactor = std::abs(NdotL * sc.NdotV) / (in.origin() - sc.P).squaredNorm();
			eyeV[eyeDepth].PDF_Backward	= sc.Material->pdf(sc, -sc.V, -sc.NdotV, session);
			eyeV[eyeDepth].SC			   = sc;

			MaterialSample ms = sc.Material->sample(
				sc, session.tile()->random().get3D(), session);
			NdotL					   = std::abs(sc.N.dot(ms.L));
			eyeV[eyeDepth].PDF_Forward = ms.safePDF_S();
			eyeV[eyeDepth].Specular	= ms.isSpecular();
			++eyeDepth;

			threadData.CumWeight *= threadData.NextWeight;
			cumPDF *= nextPDF;

			sc.Material->eval(threadData.NextWeight, sc, ms.L, NdotL, session);
			threadData.NextWeight *= /*NdotL*/ 1 / ms.safePDF_S();
			nextPDF = ms.safePDF_S() * eyeV[eyeDepth - 1].GeometricFactor;

			if (!ms.isSpecular()) {
				++diffBounces;
			}

			if (sc.Material->isLight() || threadData.NextWeight.isOnlyZero(SpecEpsilon)) {
				break;
			}

			current = current.next(sc.P, ms.L);
		} else {
			break;
		}
	}

	return eyeDepth;
}

void BiDirectIntegrator::combineLightEyePath(const Ray& in, Spectrum& spec, uint32 lightNr, uint32 eyeLength, const RenderSession& session)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];

	spec.clear();
	float pdf = 0;

	// Zero Light Patch connection (s=0,t=n)
	//MSI::balance(spec, pdf,
	//		   threadData.EyeEmissiveEnd * threadData.EyeVertices[eyeLength - 1].CumulativeWeight, /* Unweighted contribution */
	//		   threadData.EyeVertices[eyeLength - 1].CumulativePDF);

	// Other permutations (Note: the case t == 0 is skipped as the eye is not physically represent in the scene)
	for (uint32 s = 1; s < threadData.LightPathLength[lightNr]; ++s) {
		for (uint32 t = 1; t < eyeLength; ++t) {
			const float npdf = combineLightEyeNode(in, threadData.LiWeight,
												   lightNr, s, t, session);
			MSI::balance(spec, pdf, threadData.LiWeight, npdf);
		}
	}
}

float BiDirectIntegrator::combineLightEyeNode(const Ray& in, Spectrum& spec, uint32 lightNr, uint32 s, uint32 t, const RenderSession& session)
{
	BIDI_ThreadData& threadData = mThreadData[session.thread()];
	const auto& light			= threadData.LightVertices[s];
	const auto& eye				= threadData.EyeVertices[t];

	if (light.Specular || eye.Specular)
		return 0;

	const Eigen::Vector3f TS  = light.SC.P - eye.SC.P;
	const Eigen::Vector3f dir = TS.normalized();

	Ray current = Ray(in.weight(), in.pixelIndex(),
					  Ray::safePosition(eye.SC.P, dir),
					  dir,
					  0,
					  in.time(), in.wavelengthIndex(),
					  in.flags() | RF_Light);

	ShaderClosure sc;
	RenderEntity* entity = renderer()->shoot(current, sc, session);

	if (entity == light.VirtualEntity && (sc.P - light.SC.P).squaredNorm() <= LightEpsilon) {
		const float NdotL = eye.SC.N.dot(dir);
		const float NdotL2 = light.SC.N.dot(dir);
		const float G	 = std::abs(NdotL * NdotL2) / TS.squaredNorm();

		eye.SC.Material->eval(spec, eye.SC, dir, NdotL, session);
		light.SC.Material->eval(threadData.NextWeight, light.SC, -dir, -NdotL2, session);

		spec *= light.CumulativeWeight * eye.CumulativeWeight * threadData.NextWeight * G;

		return eye.PDF_Forward * eye.GeometricFactor * light.PDF_Forward * light.GeometricFactor;
	}

	return 0;
}
} // namespace PR
