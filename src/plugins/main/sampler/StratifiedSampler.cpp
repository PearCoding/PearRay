#include "SceneLoadContext.h"
#include "math/Projection.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
class PR_LIB StratifiedSampler : public ISampler {
public:
	StratifiedSampler(Random& random, uint32 samples)
		: ISampler(samples)
		, mRandom(random)
		, m2D_X(static_cast<uint32>(std::sqrt(samples)))
	{
	}

	virtual ~StratifiedSampler() = default;

	float generate1D(uint32 index) override
	{
		auto ret = Projection::stratified(mRandom.getFloat(), index, maxSamples());
		return ret;
	}

	// Need better strategy for 2D and 3D
	Vector2f generate2D(uint32 index) override
	{
		auto x = Projection::stratified(mRandom.getFloat(), index % m2D_X, maxSamples());
		auto y = Projection::stratified(mRandom.getFloat(), index / m2D_X, maxSamples());

		return Vector2f(x, y);
	}

private:
	Random& mRandom;

	uint32 m2D_X;
};

class StratifiedSamplerFactory : public ISamplerFactory {
public:
	StratifiedSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<ISampler> createInstance(Random& rnd) const override
	{
		uint32 samples = (uint32)mParams.getUInt("sample_count", 128);
		return std::make_shared<StratifiedSampler>(rnd, samples);
	}

private:
	ParameterGroup mParams;
};

class StratifiedSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<StratifiedSamplerFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "stratified" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::StratifiedSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
