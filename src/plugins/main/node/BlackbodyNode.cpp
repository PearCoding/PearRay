#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"
#include "spectral/Blackbody.h"

namespace PR {
static inline float normFactor(float temp)
{
	constexpr float WienDisplacementNM = 2.897771955e+6f;
	const float maxNM				   = WienDisplacementNM / temp;
	const float maxVal				   = blackbody(maxNM, temp);
	return 1 / maxVal;
}

// If NormalizedPeak is true, normalize it based on the maximum value such that it is 1
template <bool NormalizedPeak>
class ConstBlackbodyNode : public FloatSpectralNode {
public:
	explicit ConstBlackbodyNode(float temperature)
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mTemperature(temperature)
		, mNorm(normFactor(temperature))
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = blackbody(ctx.WavelengthNM[i], mTemperature);

		if constexpr (NormalizedPeak)
			res *= mNorm;

		return res;
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Blackbody (" << mTemperature << " K)";
		if constexpr (NormalizedPeak)
			sstream << " [Normalized]";
		return sstream.str();
	}

private:
	const float mTemperature;
	const float mNorm;
};

template <bool NormalizedPeak>
class DynamicBlackbodyNode : public FloatSpectralNode {
public:
	explicit DynamicBlackbodyNode(const std::shared_ptr<FloatScalarNode>& temperature)
		: FloatSpectralNode(temperature->flags() | NodeFlag::SpectralVarying)
		, mTemperature(temperature)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		const float temp = mTemperature->eval(ctx);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = blackbody(ctx.WavelengthNM[i], temp);

		if constexpr (NormalizedPeak)
			res *= normFactor(temp);

		return res;
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Blackbody ( T:" << mTemperature->dumpInformation() << ")";
		if constexpr (NormalizedPeak)
			sstream << " [Normalized]";
		return sstream.str();
	}

private:
	const std::shared_ptr<FloatScalarNode> mTemperature;
};

class BlackbodyPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const auto prop		  = ctx.parameters().getParameter(0);
		const bool normalized = ctx.parameters().getParameter(1).getBool(false);

		if (prop.isReference()) {
			if (normalized)
				return std::make_shared<DynamicBlackbodyNode<true>>(ctx.lookupScalarNode(prop));
			else
				return std::make_shared<DynamicBlackbodyNode<false>>(ctx.lookupScalarNode(prop));
		} else {
			if (normalized)
				return std::make_shared<ConstBlackbodyNode<true>>(prop.getNumber(6500.0f));
			else
				return std::make_shared<ConstBlackbodyNode<true>>(prop.getNumber(6500.0f));
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "blackbody" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Blackbody Node", "A temperature based blackbody node")
			.Identifiers(getNames())
			.Inputs()
			.ScalarNode("temperature", "Temperature of the blackbody in Kelvin", 6500.0f)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::BlackbodyPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)