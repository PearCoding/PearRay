#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"

namespace PR {
class IntDirect : public IIntegrator {
public:
	IntDirect(RenderContext* renderer)
		: IIntegrator(renderer)
	{
	}

	virtual ~IntDirect() = default;

	void init() override
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
		return false;
	}

	// Per thread
	void onPass(const RenderTileSession& session, uint32 pass) override
	{
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}
};

class IntDirectFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(RenderContext* ctx) override
	{
		return std::make_shared<IntDirect>(ctx);
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