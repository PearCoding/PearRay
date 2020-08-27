#include "Profiler.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include <sstream>

namespace PR {

// Special material not scattering rays and changing energy
class NullMaterial : public IMaterial {
public:
	NullMaterial(uint32 id)
		: IMaterial(id)
	{
	}

	virtual ~NullMaterial() = default;

	int flags() const override { return MF_DeltaDistribution; }

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.Weight = 1;
		out.PDF_S  = 1;
		out.Type   = MST_SpecularTransmission;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Weight = 1;
		out.PDF_S  = 1;
		out.Type   = MST_SpecularTransmission;
		out.L	   = -in.Context.V;
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
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext&)
	{
		return std::make_shared<NullMaterial>(id);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "null" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::NullMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)