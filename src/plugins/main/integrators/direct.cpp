#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "material/IMaterial.h"
#include "path/LightPath.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

namespace PR {
class IntDirect : public IIntegrator {
public:
	IntDirect()
		: IIntegrator()
	{
	}

	virtual ~IntDirect() = default;

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
		LightPath stdPath = LightPath::createCDL(1);

		while (session.handleCameraRays()) {
			session.handleHits([&](const HitEntry&,
								   const Ray& ray, const GeometryPoint& pt,
								   IEntity*, IMaterial* material) {
				session.tile()->statistics().addEntityHitCount();

				ShadingPoint spt;
				spt.Ray		 = ray;
				spt.Geometry = pt;
				spt.Ns		 = pt.N;
				spt.Nx		 = pt.Nx;
				spt.Ny		 = pt.Ny;
				spt.Radiance = 1;

				// Pick light and point
				float pdf;
				Vector3f pos;
				IEntity* light = session.pickRandomLight(pos, pdf);
				if (!light)
					return;
				Vector3f L = (pos - spt.Geometry.P).normalized();

				// Trace shadow ray
				Ray shadow			= ray.next(spt.Geometry.P, L);
				ShadowHit shadowHit = session.traceShadowRay(shadow);
				if (!shadowHit.Successful || shadowHit.EntityID != light->id())
					return;

				// Evaluate
				MaterialEvalInput in;
				in.Point	= spt;
				in.Outgoing = L;

				MaterialEvalOutput out;
				material->eval(in, out, session);

				const float cosD = std::max(0.0f, L.dot(spt.Ns));
				spt.Radiance *= out.Weight * cosD / pdf;

				session.pushFragment(ray.PixelIndex, spt, stdPath);
			});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

private:
	std::vector<LightPath> mLightPaths;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(uint32, uint32, const Environment& env) override
	{
		return std::make_shared<IntDirect>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "direct", "standard", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntDirectFactory, "int_direct", PR_PLUGIN_VERSION)