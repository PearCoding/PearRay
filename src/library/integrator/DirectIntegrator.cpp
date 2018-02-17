#include "DirectIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "math/MSI.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"
#include "sampler/RandomSampler.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

namespace PR {
struct DI_ThreadData {
	std::vector<Spectrum> Weight;
	std::vector<Spectrum> Evaluation;

	explicit DI_ThreadData(RenderContext* context)
		: Weight(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
		, Evaluation(context->settings().maxRayDepth(), Spectrum(context->spectrumDescriptor()))
	{
	}
};

DirectIntegrator::DirectIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

void DirectIntegrator::init()
{
	OnePassIntegrator::init();

	mThreadData.clear();

	for (uint32 i = 0; i < renderer()->threads(); ++i)
		mThreadData.emplace_back(renderer());
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

	RenderEntity* entity = renderer()->shootWithEmission(spec, in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded()
		|| diffbounces > renderer()->settings().maxDiffuseBounces())
		return;

	DI_ThreadData& threadData = mThreadData[session.thread()];
	const uint32 depth		  = in.depth();
	float full_pdf			  = 0;

	// Used temporary
	ShaderClosure other_sc;

	bool noSpecular = true;

	// Hemisphere sampling
	for (uint32 i = 0;
		 i < renderer()->settings().maxLightSamples() && noSpecular;
		 ++i) {
		const uint32 path_count = sc.Material->samplePathCount();
		PR_ASSERT(path_count > 0, "path_count should be always higher than 0.");

		const Eigen::Vector3f rnd = session.tile()->random().get3D();

		for (uint32 path = 0; path < path_count; ++path) {
			MaterialSample ms = sc.Material->samplePath(sc, rnd, path, session);
			const float NdotL = std::abs(ms.L.dot(sc.N));

			if (ms.PDF_S <= PR_EPSILON || NdotL <= PR_EPSILON)
				continue;

			applyRay(threadData.Weight[depth], in.next(sc.P, ms.L), session,
					 !std::isinf(ms.PDF_S) ? diffbounces + 1 : diffbounces,
					 other_sc);

			//if (!weight.isOnlyZero()) {
			sc.Material->eval(threadData.Evaluation[depth], sc, ms.L, NdotL, session);
			threadData.Weight[depth] *= threadData.Evaluation[depth] * NdotL;
			MSI::balance(spec, full_pdf, threadData.Weight[depth], ms.PDF_S);
			//}

			if (ms.isSpecular())
				noSpecular = false;
		}
	}

	if (noSpecular) {
		// Area sampling!
		for (RenderEntity* light : renderer()->lights()) {
			for (uint32 i = 0; i < renderer()->settings().maxLightSamples(); ++i) {
				const Eigen::Vector3f rnd		  = session.tile()->random().get3D();
				RenderEntity::FacePointSample fps = light->sampleFacePoint(rnd);

				const Eigen::Vector3f PS = fps.Point.P - sc.P;
				const Eigen::Vector3f L  = PS.normalized();
				const float NdotL		 = std::abs(L.dot(sc.N)); // No back light detection

				float pdfA = fps.PDF_A;

				if (pdfA <= PR_EPSILON || NdotL <= PR_EPSILON)
					continue;

				Ray ray = in.next(sc.P, L);
				ray.setFlags(ray.flags() | RF_Light);

				// Full light!!
				if (renderer()->shootWithEmission(threadData.Weight[depth], ray, other_sc, session) == light /*&& (fps.Point.P - other_sc.P).squaredNorm() <= LightEpsilon*/) {
					if (other_sc.Flags & SCF_Inside) // Wrong side (Back side)
						continue;

					sc.Material->eval(threadData.Evaluation[depth], sc, L, NdotL, session);
					threadData.Weight[depth] *= threadData.Evaluation[depth] * NdotL * light->surfaceArea(fps.Point.Material);

					const float pdfS = MSI::toSolidAngle(pdfA, PS.squaredNorm(), std::abs(sc.NdotV * NdotL));
					MSI::balance(spec, full_pdf, threadData.Weight[depth], pdfS);
				}
			}
		}

		float inf_pdf;
		handleInfiniteLights(threadData.Weight[depth], in, sc, session, inf_pdf);
		MSI::balance(spec, full_pdf, threadData.Weight[depth], inf_pdf);
	}
}
} // namespace PR
