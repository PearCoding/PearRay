#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "container/CSV.h"
#include "shader/EquidistantSpectrumNode.h"
#include "shader/INodePlugin.h"

namespace PR {
class SPDNode : public EquidistantSpectrumNode {
public:
	inline SPDNode(const EquidistantSpectrum& spectrum)
		: EquidistantSpectrumNode(spectrum)
	{
	}
};

class SPDFilePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		std::string file = ctx.parameters().getString("file", "");
		if (file.empty())
			file = ctx.parameters().getString(0, "");

		if (file.empty()) {
			PR_LOG(L_ERROR) << "No file given for SPD" << std::endl;
			return nullptr;
		}

		CSV csv(ctx.escapePath(file));

		if (!csv.isValid()) {
			PR_LOG(L_ERROR) << "Given file '" << file << "' is not valid" << std::endl;
			return nullptr;
		}

		if (csv.columnCount() <= 1 || csv.rowCount() < 2) {
			PR_LOG(L_ERROR) << "Given file '" << file << "' has not enough data" << std::endl;
			return nullptr;
		}

		const float delta = csv(1, 0) - csv(0, 0);
		const float start = csv(0, 0);
		const float end	  = csv(csv.rowCount() - 1, 0);

		if (delta <= PR_EPSILON) {
			PR_LOG(L_ERROR) << "Given file '" << file << "' has invalid wavelengths" << std::endl;
			return nullptr;
		}

		bool hasNonEquidistantData = false;

		// Extract optional column index
		size_t dataColumn = 1;
		if (ctx.parameters().hasParameter("column"))
			dataColumn = ctx.parameters().getUInt("column", dataColumn);

		if (ctx.parameters().positionalParameterCount() > 1)
			dataColumn = ctx.parameters().getUInt(1, dataColumn);

		dataColumn = std::min(dataColumn, csv.columnCount());

		// Set optional norm factor
		float normF = 1;
		if (ctx.parameters().hasParameter("percentage"))
			normF = ctx.parameters().getBool("percentage", false) ? (1 / 100.0f) : 1.0f;

		if (ctx.parameters().positionalParameterCount() > 2)
			normF = ctx.parameters().getBool(2, false) ? (1 / 100.0f) : 1.0f;

		// Build equidistant spectrum
		EquidistantSpectrum spectrum(csv.rowCount(), start, end);
		for (size_t i = 0; i < csv.rowCount(); ++i) {
			const float expectedWvl = i * delta + start;
			if (std::abs(csv(i, 0) - expectedWvl) > PR_EPSILON)
				hasNonEquidistantData = true;
			spectrum.at(i) = std::max(0.0f, csv(i, dataColumn) * normF);
		}

		if (hasNonEquidistantData)
			PR_LOG(L_WARNING) << "Given file '" << file << "' has non equidistant data" << std::endl;

		return std::make_shared<SPDNode>(spectrum);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "spd" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("SPD Node", "Spectral power distribution file")
			.Identifiers(getNames())
			.Inputs()
			.Filename("file", "Path to a CSV file containing wavelength and power distribution")
			.UInt("column", "Index of column the data is in. Column 0 is reserved for wavelengths", 1, 0, 16, 0, 1024, true)
			.Bool("percentage", "Data given as percentage, divide by 100 ", false, true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SPDFilePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)