#include "SceneLoadContext.h"
#include "math/Projection.h"

#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "sampler/ISamplerPlugin.h"
#include <vector>

namespace PR {
constexpr uint32 DEF_SAMPLE_COUNT = 128;

class PR_LIB_BASE StratifiedSampler : public ISampler {
public:
	StratifiedSampler(Random& random, uint32 samples, uint32 groups)
		: ISampler(samples)
		, mRandom(random)
		, m2D_X(static_cast<uint32>(std::sqrt(groups)))
		, mGroups(groups)
	{
	}

	virtual ~StratifiedSampler() = default;

	float generate1D(uint32 index) override
	{
		auto ret = Projection::stratified(mRandom.getFloat(), index, mGroups);
		return ret;
	}

	// Need better strategy for 2D and 3D
	Vector2f generate2D(uint32 index) override
	{
		auto x = Projection::stratified(mRandom.getFloat(), index % m2D_X, m2D_X);
		auto y = Projection::stratified(mRandom.getFloat(), index / m2D_X, m2D_X);

		return Vector2f(x, y);
	}

private:
	Random& mRandom;

	const uint32 m2D_X;
	const uint32 mGroups;
};

class StratifiedSamplerFactory : public ISamplerFactory {
public:
	explicit StratifiedSamplerFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	uint32 requestedSampleCount() const override
	{
		return mParams.getUInt("sample_count", DEF_SAMPLE_COUNT);
	}

	std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random& rnd) const override
	{
		return std::make_shared<StratifiedSampler>(rnd, sample_count, mParams.getUInt("bins", std::max(1u, sample_count)));
	}

private:
	ParameterGroup mParams;
};

class StratifiedSamplerPlugin : public ISamplerPlugin {
public:
	std::shared_ptr<ISamplerFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<StratifiedSamplerFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "stratified" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Stratified Sampler", "A progressive sampler with medium quality")
			.Identifiers(getNames())
			.Inputs()
			.UInt("sample_count", "Sample count requested", DEF_SAMPLE_COUNT)
			.UInt("bins", "Bins used for jittering", DEF_SAMPLE_COUNT)
			.Specification()
			.get();
	}
	
	
};
} // namespace PR

PR_PLUGIN_INIT(PR::StratifiedSamplerPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
