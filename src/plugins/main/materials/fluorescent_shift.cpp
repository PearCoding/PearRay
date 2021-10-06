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
class FluorescentShift : public IMaterial {
public:
	FluorescentShift(const std::shared_ptr<FloatSpectralNode>& alb, const std::shared_ptr<FloatSpectralNode>& shift)
		: IMaterial()
		, mAlbedo(alb)
		, mShift(shift)
	{
	}

	virtual ~FluorescentShift() = default;

	inline static float culling(float u)
	{
		if constexpr (TwoSided)
			return std::abs(u);
		else
			return std::max(0.0f, u);
	}

	// The stokes shift is not linear in the wavelength domain
	// Note: stokesShift(stokesShift(a, s), -s) = a
	inline static SpectralBlob stokesShift(const SpectralBlob& wvl, const SpectralBlob& shift)
	{
		/* E = hc/w -> K = 1/w <=> w = 1/K
		 * Sw = 1/(K - S) <=> Sw = 1/(1/w - S) <=> Sw = w / (1 - w*S)
		 * The shift variable (S) is given in 1/nm units
		 * Keep in mind, lower energy means larger wavelength
		 */
		return wvl / (1 - wvl * shift);
	}

	MaterialFlags flags() const override { return MaterialFlag::HasFluorescence; }

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const bool isLight				= in.Context.RayFlags & RayFlag::Light;
		const SpectralBlob expShift		= mShift->eval(in.ShadingContext);
		const SpectralBlob expWvl		= isLight ? stokesShift(in.Context.WavelengthNM, expShift) : stokesShift(in.Context.WavelengthNM, -expShift);
		const SpectralBlob actShift		= in.Context.FluorescentWavelengthNM - expWvl;
		const SpectralBlob fluorescentW = (actShift < 0).select(0, (1 - actShift / expShift).cwiseMax(0.0f));

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.Weight		= fluorescentW * mAlbedo->eval(in.ShadingContext) * dot * PR_INV_PI;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Type		= MaterialScatteringType::DiffuseReflection;
		out.Flags		= MaterialSampleFlag::Fluorescent;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Flags		= MaterialSampleFlag::Fluorescent;
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

		out.IntegralWeight = mAlbedo->eval(in.ShadingContext);

		// We apply the forward stokes shift for lights and reverse stokes shift for camera lights
		if (in.Context.RayFlags & RayFlag::Light)
			out.FluorescentWavelengthNM = stokesShift(in.Context.WavelengthNM, mShift->eval(in.ShadingContext));
		else
			out.FluorescentWavelengthNM = stokesShift(in.Context.WavelengthNM, -mShift->eval(in.ShadingContext));

		out.PDF_S = Sampling::cos_hemi_pdf(out.L(2));
		out.Type  = MaterialScatteringType::DiffuseReflection;

		// Make sure the output direction is on the same side
		out.L = in.Context.V.makeSameHemisphere(out.L);

		out.Flags = MaterialSampleFlag::Fluorescent;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <FluorescentShift>:" << std::endl
			   << "    Albedo:   " << mAlbedo->dumpInformation() << std::endl
			   << "    Shift:   " << mShift->dumpInformation() << std::endl
			   << "    TwoSided: " << (TwoSided ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
	const std::shared_ptr<FloatSpectralNode> mShift;
};

inline constexpr float stokesShiftDelta(float nm, float nm_shift) { return 1 / nm - 1 / (nm + nm_shift); }

class FluorescentShiftMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		// Calculate the stokes shift from 550nm to 552nm
		constexpr float def_shift = stokesShiftDelta(550, 2);

		const auto albedo = ctx.lookupSpectralNode({ "albedo", "base", "diffuse" }, 1);
		const auto shift  = ctx.lookupSpectralNode("shift", def_shift);

		if (ctx.parameters().getBool("two_sided", true))
			return std::make_shared<FluorescentShift<true>>(albedo, shift);
		else
			return std::make_shared<FluorescentShift<false>>(albedo, shift);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "stokes", "fluorescent_shift", "stokes_shift" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Fluorescent Diffuse BSDF", "A perfect diffuse BSDF with fluorescent properties based on the stokes shift")
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

PR_PLUGIN_INIT(PR::FluorescentShiftMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)