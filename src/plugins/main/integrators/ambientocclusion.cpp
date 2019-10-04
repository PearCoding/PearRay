#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "math/Projection.h"
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
					Eigen::Vector2f rnd  = random.get2D();
					Eigen::Vector3f dir  = Projection::hemi(rnd(0), rnd(1), pdf);
					Eigen::Vector3f ndir = Projection::tangent_align(
						Eigen::Vector3f(pt.Ng[0], pt.Ng[1], pt.Ng[2]),
						Eigen::Vector3f(pt.Nx[0], pt.Nx[1], pt.Nx[2]),
						Eigen::Vector3f(pt.Ny[0], pt.Ny[1], pt.Ny[2]),
						dir);

					Eigen::Vector3f no = Projection::safePosition(
						Eigen::Vector3f(pt.P[0], pt.P[1], pt.P[2]),
						ndir);

					Ray n = ray.next(no(0), no(1), no(2),
									 ndir(0), ndir(1), ndir(2));

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
		size_t sample_count = 100;
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