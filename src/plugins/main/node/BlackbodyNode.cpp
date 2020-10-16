#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"
#include "spectral/Blackbody.h"

namespace PR {
class BlackbodyNode : public FloatSpectralNode {
public:
	explicit BlackbodyNode(float temperature)
		: mTemperature(temperature)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = blackbody(ctx.WavelengthNM[i], mTemperature);
		return res;
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Blackbody (" << mTemperature << " K)";
		return sstream.str();
	}

private:
	const float mTemperature;
};

class BlackbodyPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		float temperature = ctx.parameters().getParameter(0).getNumber(6500.0f);
		return std::make_shared<BlackbodyNode>(temperature);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "blackbody" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::BlackbodyPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)