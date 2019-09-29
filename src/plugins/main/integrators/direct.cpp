#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "path/LightPath.h"
#include "renderer/RenderTileSession.h"
#include "renderer/RenderTile.h"

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
			session.handleHits([&](const HitEntry& entry, IEntity* entity, IMaterial* material) {
				session.tile()->statistics().addEntityHitCount();
			});
			//PR_LOG(L_INFO) << "HELLO??" << std::endl;
			// Setup
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