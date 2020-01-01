#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "path/LightPath.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

namespace PR {
class IntAO : public IIntegrator {
public:
	explicit IntAO(size_t sample_count)
		: IIntegrator()
		, mSampleCount(sample_count)
	{
	}

	virtual ~IntAO() = default;

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		PR_PROFILE_THIS;

		Random& random	= session.tile()->random();
		LightPath stdPath = LightPath::createCDL(1);

		while (session.handleCameraRays()) {
			session.handleHits(
				[&](size_t, const Ray&) {
					session.tile()->statistics().addBackgroundHitCount();
				},
				[&](const HitEntry&,
					const Ray& ray, const GeometryPoint& pt,
					IEntity* entity, IMaterial*) {
					PR_PROFILE_THIS;
					session.tile()->statistics().addEntityHitCount();

					ShadingPoint spt;
					spt.setByIdentity(ray, pt);
					spt.EntityID = entity->id();

					size_t occlusions = 0;
					float pdf;
					for (size_t i = 0; i < mSampleCount; ++i) {
						Vector2f rnd  = random.get2D();
						Vector3f dir  = Projection::hemi(rnd(0), rnd(1), pdf);
						Vector3f ndir = Tangent::fromTangentSpace(spt.N, spt.Nx, spt.Ny,
																  dir);

						Ray n = ray.next(pt.P, ndir);

						ShadowHit hit = session.traceShadowRay(n);
						if (hit.Successful)
							++occlusions;
					}

					ColorTriplet radiance = ColorTriplet::Ones();
					radiance *= 1.0f - occlusions / (float)mSampleCount;

					session.pushSPFragment(spt, stdPath);
					session.pushSpectralFragment(radiance, ray, stdPath);
				});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

private:
	size_t mSampleCount;
};

class IntAOFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(uint32, uint32, const SceneLoadContext& ctx) override
	{
		const Registry& reg = ctx.Env->registry();
		size_t sample_count = (size_t)reg.getByGroup<uint32>(RG_INTEGRATOR, "ao/sample_count", 10);

		return std::make_shared<IntAO>(sample_count);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "ao", "occlusion", "ambient_occlusion" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntAOFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)