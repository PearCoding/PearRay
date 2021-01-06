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
class LambertMaterial : public IMaterial {
public:
	LambertMaterial(const std::shared_ptr<FloatSpectralNode>& alb)
		: IMaterial()
		, mAlbedo(alb)
	{
	}

	virtual ~LambertMaterial() = default;

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

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.Weight		= mAlbedo->eval(in.ShadingContext) * dot * PR_INV_PI;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
		out.Type		= MaterialScatteringType::DiffuseReflection;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = in.Context.V.sameHemisphere(in.Context.L) ? culling(in.Context.NdotL()) : 0;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
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

		out.Weight = mAlbedo->eval(in.ShadingContext) * out.L(2) * PR_INV_PI;
		out.PDF_S  = Sampling::cos_hemi_pdf(out.L(2));
		out.Type   = MaterialScatteringType::DiffuseReflection;

		// Make sure the output direction is on the same side
		out.L = in.Context.V.makeSameHemisphere(out.L);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DiffuseMaterial>:" << std::endl
			   << "    Albedo:   " << mAlbedo->dumpInformation() << std::endl
			   << "    TwoSided: " << (TwoSided ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
};

class LambertMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		if (ctx.parameters().getBool("two_sided", true))
			return std::make_shared<LambertMaterial<true>>(ctx.lookupSpectralNode("albedo", 1));
		else
			return std::make_shared<LambertMaterial<false>>(ctx.lookupSpectralNode("albedo", 1));
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "diffuse", "lambert" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Diffuse BSDF", "A perfect diffuse BSDF")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected", 1.0f)
			.Bool("two_sided", "Specify BSDF as two sided", true)
			.Specification()
			.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::LambertMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)