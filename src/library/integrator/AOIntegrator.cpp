#include "AOIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "math/Projection.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"
#include "sampler/RandomSampler.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"

namespace PR {
// Registry Entries:
static const char* RE_SAMPLES		= "ao/sample_count";
static const char* RE_USE_MATERIALS = "ao/use_materials";

AOIntegrator::AOIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
	, mMaxAOSamples(0)
	, mUseMaterials(true)
{
}

AOIntegrator::~AOIntegrator()
{
}

void AOIntegrator::init()
{
	OnePassIntegrator::init();

	mMaxAOSamples = std::max<uint32>(1,
									 renderer()->registry()->getByGroup<uint32>(RG_INTEGRATOR,
																				RE_SAMPLES,
																				1));
	mUseMaterials = renderer()->registry()->getByGroup<bool>(RG_INTEGRATOR,
															 RE_USE_MATERIALS,
															 true);
}

void AOIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	spec.clear();

	const RenderEntity* entity = renderer()->shoot(in, sc, session);

	if (!entity || !sc.Material
		|| !sc.Material->canBeShaded())
		return;

	float hits = 0;
	if (mUseMaterials) {
		for (uint32 i = 0; i < mMaxAOSamples; ++i) {
			const Eigen::Vector3f rnd_hemi = session.tile()->random().get3D();
			const uint32 path_count		   = sc.Material->samplePathCount();
			bool specular				   = false;

			for (uint32 path = 0; path < path_count; ++path) {
				const MaterialSample ms = sc.Material->samplePath(sc, rnd_hemi, path, session);

				specular = specular || ms.isSpecular();

				if (ms.PathWeight <= PR_EPSILON) {
					continue;
				}

				if (renderer()->shootForDetection(in.next(sc.P, ms.L), session)) {
					hits += ms.PathWeight;
				}
			}

			if (specular)
				break;
		}
	} else {
		for (uint32 i = 0; i < mMaxAOSamples; ++i) {
			const Eigen::Vector3f rnd = session.tile()->random().get3D();
			float pdf;
			const Eigen::Vector3f L = Projection::tangent_align(sc.N, sc.Nx, sc.Ny,
																Projection::hemi(rnd(0), rnd(1),
																				 pdf));

			if (renderer()->shootForDetection(in.next(sc.P, L), session)) {
				hits += 1.0f;
			}
		}
	}

	if (mMaxAOSamples > 0) {
		spec.fill(1 - hits / (float)mMaxAOSamples);
	}
}

} // namespace PR
