#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "path/LightPath.h"
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

	void init(RenderContext* renderer) override
	{
	}

	void onStart() override
	{
	}

	void onNextPass(uint32 i, bool& clean) override
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
	void onPass(RenderTileSession& session, uint32 pass) override
	{
		while (session.handleCameraRays()) {
			session.handleHits([&](const HitEntry& entry,
								   const Ray& ray, const GeometryPoint& pt,
								   IEntity* entity, IMaterial* material) {
				session.tile()->statistics().addEntityHitCount();

				ShadingPoint spt;
				spt.Ray		 = ray;
				spt.Geometry = pt;

				spt.Radiance = 1;

				session.pushFragment(ray.PixelIndex, spt);
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
	std::shared_ptr<IIntegrator> create() override
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