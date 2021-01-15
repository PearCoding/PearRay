#include "Profiler.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include <sstream>

namespace PR {

// Special material not scattering rays and changing energy
// TODO: The integrators should not apply delta stuff on this, as its not 'wavelength' dependent
class NullMaterial : public IMaterial {
public:
	explicit NullMaterial()
		: IMaterial()
	{
	}

	virtual ~NullMaterial() = default;

	MaterialFlags flags() const override { return MaterialFlag::OnlyDeltaDistribution; }

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S  = 0.0f;
		out.Type   = MaterialScatteringType::SpecularTransmission;
		out.Weight = SpectralBlob::Zero();
		out.Flags  = MaterialSampleFlag::Null | MaterialSampleFlag::DeltaDistribution;
	}

	void pdf(const MaterialEvalInput&, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");
		out.PDF_S = 0.0f;
		out.Flags = MaterialSampleFlag::Null | MaterialSampleFlag::DeltaDistribution;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Weight = 1;
		out.PDF_S  = 1;
		out.Type   = MaterialScatteringType::SpecularTransmission;
		out.L	   = -in.Context.V;
		out.Flags  = MaterialSampleFlag::Null | MaterialSampleFlag::DeltaDistribution;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IMaterial::dumpInformation()
			   << "  <NullMaterial>:" << std::endl;

		return stream.str();
	}
};

class NullMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext&) override
	{
		return std::make_shared<NullMaterial>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "null" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Null BSDF", "Does nothing, is nothing.")
			.Identifiers(getNames())
			.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::NullMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)