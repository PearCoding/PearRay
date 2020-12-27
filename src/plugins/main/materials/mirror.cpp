#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Projection.h"
#include "math/Scattering.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class MirrorMaterial : public IMaterial {
public:
	MirrorMaterial(const std::shared_ptr<FloatSpectralNode>& alb)
		: IMaterial()
		, mSpecularity(alb)
	{
	}

	virtual ~MirrorMaterial() = default;

	int flags() const override { return MF_OnlyDeltaDistribution; }

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S  = 0.0f;
		out.Type   = MST_SpecularReflection;
		out.Weight = SpectralBlob::Zero();
		out.Flags  = MSF_DeltaDistribution;
	}

	void pdf(const MaterialEvalInput&, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S = 0;
		out.Flags  = MSF_DeltaDistribution;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.Weight = mSpecularity->eval(in.ShadingContext);
		out.Type   = MST_SpecularReflection;
		out.PDF_S  = 1;
		out.L	   = Scattering::reflect(in.Context.V);
		out.Flags  = MSF_DeltaDistribution;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <MirrorMaterial>:" << std::endl
			   << "    Specularity: " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
};

class MirrorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		return std::make_shared<MirrorMaterial>(ctx.lookupSpectralNode("specularity", 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "mirror", "reflection" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MirrorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)