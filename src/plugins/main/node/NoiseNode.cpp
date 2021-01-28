#include "Environment.h"
#include "Logger.h"
#include "Random.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "renderer/RenderContext.h"
#include "shader/INodePlugin.h"

namespace PR {
class NoiseContext {
public:
	inline explicit NoiseContext(size_t seed)
		: mSeed(seed)
	{
	}

	inline size_t seed() const { return mSeed; }
	inline Random& random(size_t thread_id)
	{
		PR_ASSERT(thread_id < mRandoms.size(), "Invalid thread index");
		return mRandoms[thread_id];
	}

	inline void setupThreadData(size_t thread_count)
	{
		mRandoms.reserve(thread_count);
		for (size_t i = 0; i < thread_count; ++i)
			mRandoms.emplace_back(mSeed);
	}

private:
	std::vector<Random> mRandoms;
	const size_t mSeed;
};

class NoiseNode : public FloatScalarNode {
public:
	NoiseNode(const std::shared_ptr<ServiceObserver>& so, size_t seed)
		: FloatScalarNode(0 /*Or all?*/)
		, mContext(seed)
		, mServiceObserver(so)
	{
		if (mServiceObserver)
			mCBID = mServiceObserver->registerBeforeRender([this](RenderContext* ctx) {
				mContext.setupThreadData(ctx->threadCount());
			});
	}

	virtual ~NoiseNode()
	{
		if (mServiceObserver)
			mServiceObserver->unregister(mCBID);
	}

	float eval(const ShadingContext& ctx) const override
	{
		return mContext.random(ctx.ThreadIndex).getFloat();
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Noise (" << mContext.seed() << ")";
		return sstream.str();
	}

private:
	mutable NoiseContext mContext;

	const std::shared_ptr<ServiceObserver> mServiceObserver;
	ServiceObserver::CallbackID mCBID;
};

constexpr size_t DEFAULT_SEED = 4207337;

class NoideNodePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const size_t seed = ctx.parameters().getUInt(0, DEFAULT_SEED);
		return std::make_shared<NoiseNode>(ctx.environment()->serviceObserver(), seed);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "noise", "random", "rnd" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Noise Node", "A node generating random numbers between [0, 1]")
			.Identifiers(getNames())
			.Inputs()
			.Int("seed", "Seed", DEFAULT_SEED)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::NoideNodePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)