#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool TwoSided>
class FlourescentDiffuse : public IMaterial {
public:
	FlourescentDiffuse(const std::shared_ptr<FloatSpectralNode>& alb, const std::shared_ptr<FloatSpectralNode>& shift)
		: IMaterial()
		, mAlbedo(alb)
		, mShift(shift)
	{
	}

	virtual ~FlourescentDiffuse() = default;

	inline static float culling(float u)
	{
		if constexpr (TwoSided)
			return std::abs(u);
		else
			return std::max(0.0f, u);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const SpectralBlob expShift		= mShift->eval(in.ShadingContext);
		const SpectralBlob actShift		= in.Context.FlourescentWavelengthNM - in.Context.WavelengthNM;
		const SpectralBlob flourescentW = (actShift < 0).select(0, (1 - actShift / expShift).cwiseMax(0.0f));

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.Weight		= flourescentW * mAlbedo->eval(in.ShadingContext) * dot * PR_INV_PI;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Type		= MaterialScatteringType::DiffuseReflection;
		out.Flags		= MaterialSampleFlag::Flourescent;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Flags		= MaterialSampleFlag::Flourescent;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if constexpr (!TwoSided) {
			if (in.Context.NdotV() < 0.0f) {
				out = MaterialSampleOutput::Reject(MaterialScatteringType::DiffuseReflection);
				return;
			}
		}

		out.L = Sampling::cos_hemi(in.RND.getFloat(), in.RND.getFloat());

		out.Weight					= mAlbedo->eval(in.ShadingContext) * out.L(2) * PR_INV_PI;
		out.FlourescentWavelengthNM = in.Context.WavelengthNM + mShift->eval(in.ShadingContext);
		out.PDF_S					= Sampling::cos_hemi_pdf(out.L(2));
		out.Type					= MaterialScatteringType::DiffuseReflection;

		// Make sure the output direction is on the same side
		out.L = in.Context.V.makeSameHemisphere(out.L);

		out.Flags = MaterialSampleFlag::Flourescent;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <FlourescentDiffuse>:" << std::endl
			   << "    Albedo:   " << mAlbedo->dumpInformation() << std::endl
			   << "    Shift:   " << mShift->dumpInformation() << std::endl
			   << "    TwoSided: " << (TwoSided ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
	const std::shared_ptr<FloatSpectralNode> mShift;
};

class FlourescentDiffuseMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const auto albedo = ctx.lookupSpectralNode({ "albedo", "base", "diffuse" }, 1);
		const auto shift  = ctx.lookupSpectralNode("shift", 1); // Shift of 1 nm

		if (ctx.parameters().getBool("two_sided", true))
			return std::make_shared<FlourescentDiffuse<true>>(albedo, shift);
		else
			return std::make_shared<FlourescentDiffuse<false>>(albedo, shift);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "flourescent", "flourescent_diffuse" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Flourescent Diffuse BSDF", "A perfect diffuse BSDF with flourescent properties")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected", 1.0f)
			.SpectralNode("shift", "Wavelength shift given in nano meters", 1.0f)
			.Bool("two_sided", "Specify BSDF as two sided", true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FlourescentDiffuseMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)