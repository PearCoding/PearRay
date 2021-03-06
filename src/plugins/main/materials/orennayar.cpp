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

class OrenNayarMaterial : public IMaterial {
public:
	OrenNayarMaterial(const std::shared_ptr<FloatSpectralNode>& alb,
					  const std::shared_ptr<FloatScalarNode>& rough)
		: IMaterial()
		, mAlbedo(alb)
		, mRoughness(rough)
	{
	}

	virtual ~OrenNayarMaterial() = default;

	// https://mimosa-pudica.net/improved-oren-nayar.html
	inline SpectralBlob calc(const Vector3f& L, float NdotL, const MaterialSampleContext& ctx, const ShadingContext& sctx) const
	{
		float roughness = mRoughness->eval(sctx);
		roughness *= roughness;

		SpectralBlob weight = mAlbedo->eval(sctx);

		if (roughness > PR_EPSILON) {
			const float s = -NdotL * ctx.NdotV() + ctx.V.dot(L);
			const float t = s < PR_EPSILON ? 1.0f : std::max(NdotL, ctx.NdotV());

			const SpectralBlob A = SpectralBlob::Ones() * (1 - 0.5f * roughness / (roughness + 0.33f))
								   + 0.17f * weight * roughness / (roughness + 0.13f);
			const float B = 0.45f * roughness / (roughness + 0.09f);
			weight *= A + SpectralBlob::Ones() * (B * s / t);
		}

		return weight;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::max(0.0f, in.Context.NdotL());
		out.Weight		= calc(in.Context.L, dot, in.Context, in.ShadingContext) * PR_INV_PI * dot;
		out.Type		= MaterialScatteringType::DiffuseReflection;
		out.PDF_S		= Sampling::cos_hemi_pdf(dot);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float dot = std::max(0.0f, in.Context.NdotL());
		out.PDF_S		= Sampling::cos_hemi_pdf(dot) * PR_INV_PI * dot;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.L = Sampling::cos_hemi(in.RND.getFloat(), in.RND.getFloat());

		float NdotL		   = std::max(0.0f, out.L(2));
		out.IntegralWeight = calc(out.L, NdotL, in.Context, in.ShadingContext);
		out.Type		   = MaterialScatteringType::DiffuseReflection;
		out.PDF_S		   = Sampling::cos_hemi_pdf(out.L(2));
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <OrenNayarMaterial>:" << std::endl
			   << "    Albedo: " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Roughness: " << (mRoughness ? mRoughness->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mAlbedo;
	std::shared_ptr<FloatScalarNode> mRoughness;
};

class OrenNayarMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<OrenNayarMaterial>(ctx.lookupSpectralNode("albedo", 1),
												   ctx.lookupScalarNode("roughness", 0.5f));
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "orennayar", "oren", "rough" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("OrenNayar BSDF", "A not so perfectly diffuse BSDF")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected", 1.0f)
			.ScalarNode("roughness", "Roughness", 0.5f)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::OrenNayarMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)