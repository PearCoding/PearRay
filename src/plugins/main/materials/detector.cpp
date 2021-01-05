#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool HasTransmission>
class DetectorMaterial : public IMaterial {
public:
	DetectorMaterial(const std::shared_ptr<FloatSpectralNode>& frontrefl, const std::shared_ptr<FloatSpectralNode>& fronttrans,
					 const std::shared_ptr<FloatSpectralNode>& backrefl, const std::shared_ptr<FloatSpectralNode>& backtrans)
		: IMaterial()
		, mFrontReflection(frontrefl)
		, mFrontTransmission(fronttrans)
		, mBackReflection(backrefl)
		, mBackTransmission(backtrans)
	{
	}

	virtual ~DetectorMaterial() = default;

	inline SpectralBlob calc(const ShadingVector& V, const ShadingVector& L, const ShadingContext& sctx) const
	{
		if (V.isPositiveHemisphere() && V.sameHemisphere(L))
			return mFrontReflection->eval(sctx);
		else if (V.isPositiveHemisphere() && !V.sameHemisphere(L))
			return mFrontTransmission->eval(sctx);
		else if (!V.isPositiveHemisphere() && V.sameHemisphere(L))
			return mBackReflection->eval(sctx);
		else
			return mBackTransmission->eval(sctx);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::abs(in.Context.NdotL());
		out.Weight		= PR_INV_PI * calc(in.Context.V, in.Context.L, in.ShadingContext) * dot;
		out.Type		= MaterialScatteringType::DiffuseReflection;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::abs(in.Context.NdotL());
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.L = Sampling::cos_hemi(in.RND.getFloat(), in.RND.getFloat());
		out.L = in.Context.V.makeSameHemisphere(out.L);

		if constexpr (HasTransmission) {
			if (in.RND.getFloat() < 0.5f)
				out.L = -out.L;
		}

		const float NdotL = std::abs(out.L(2));
		out.Weight		  = PR_INV_PI * calc(in.Context.V, out.L, in.ShadingContext) * NdotL;
		out.PDF_S		  = Sampling::cos_hemi_pdf(NdotL);

		if (out.L[2] < 0)
			out.Type = MaterialScatteringType::DiffuseTransmission;
		else
			out.Type = MaterialScatteringType::DiffuseReflection;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DetectorMaterial>:" << std::endl
			   << "    FrontReflection:   " << mFrontReflection->dumpInformation() << std::endl
			   << "    FrontTransmission: " << mFrontTransmission->dumpInformation() << std::endl
			   << "    BackReflection:    " << mBackReflection->dumpInformation() << std::endl
			   << "    BackTransmission:  " << mBackTransmission->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mFrontReflection;
	std::shared_ptr<FloatSpectralNode> mFrontTransmission;
	std::shared_ptr<FloatSpectralNode> mBackReflection;
	std::shared_ptr<FloatSpectralNode> mBackTransmission;
};

class DetectorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const auto frontrefl  = ctx.lookupSpectralNode({ "front", "front_reflection" }, 1);
		const auto fronttrans = ctx.lookupSpectralNode("front_transmission", 0.75f);
		const auto backrefl	  = ctx.lookupSpectralNode({ "back", "back_reflection" }, 0.5f);
		const auto backtrans  = ctx.lookupSpectralNode("back_transmission", 0.25f);

		const bool hasTransmission = ctx.parameters().hasParameter("front_transmission") || ctx.parameters().hasParameter("back_transmission");

		if (hasTransmission)
			return std::make_shared<DetectorMaterial<true>>(frontrefl, fronttrans, backrefl, backtrans);
		else
			return std::make_shared<DetectorMaterial<false>>(frontrefl, fronttrans, backrefl, backtrans);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "detector" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Detector BSDF", "Based on incoming vector categorization calculated reflection")
			.Identifiers(getNames())
			.Inputs()
			.BeginBlock("Front")
			.SpectralNode("front_reflection", "Front reflection value", 1)
			.SpectralNode("front_transmission", "Front transmission value", 0.75f)
			.EndBlock()
			.BeginBlock("Back")
			.SpectralNode("back_reflection", "Back reflection value", 0.5f)
			.SpectralNode("back_transmission", "Back transmission value", 0.25f)
			.EndBlock()
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DetectorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)