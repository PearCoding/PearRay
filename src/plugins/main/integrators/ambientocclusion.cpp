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
	IntAO(size_t sample_count)
		: IIntegrator()
		, mSampleCount(sample_count)
	{
	}

	virtual ~IntAO() = default;

	void init(RenderContext*) override
	{
	}

	void onStart() override
	{
	}

	void onNextPass(uint32, bool&) override
	{
	}

	void onEnd() override
	{
	}

	bool needNextPass(uint32 i) const override
	{
		return i == 0;
	}

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		Random& random = session.tile()->random();

		while (session.handleCameraRays()) {
			session.handleHits([&](const HitEntry&,
								   const Ray& ray, const GeometryPoint& pt,
								   IEntity*, IMaterial*) {
				session.tile()->statistics().addEntityHitCount();

				size_t occlusions = 0;
				float pdf;
				for (size_t i = 0; i < mSampleCount; ++i) {
					Vector2f rnd  = random.get2D();
					Vector3f dir  = Projection::hemi(rnd(0), rnd(1), pdf);
					Vector3f ndir = Tangent::align(pt.Ng, pt.Nx, pt.Ny,
												   dir);

					Ray n = ray.next(pt.P, ndir);

					ShadowHit hit = session.traceShadowRay(n);
					if (hit.Successful)
						++occlusions;
				}

				ShadingPoint spt;
				spt.Ray		 = ray;
				spt.Geometry = pt;

				spt.Radiance = 1 - occlusions / (float)mSampleCount;

				session.pushFragment(ray.PixelIndex, spt);
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
	std::shared_ptr<IIntegrator> create() override
	{
		size_t sample_count = 20;
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

PR_PLUGIN_INIT(PR::IntAOFactory, "int_ambientocclusion", PR_PLUGIN_VERSION)