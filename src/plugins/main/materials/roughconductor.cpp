#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/MicrofacetReflection.h"
#include "math/Projection.h"
#include "math/Scattering.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {
template <bool UseVNDF, bool IsAnisotropic>
class RoughConductorMaterial : public IMaterial {
public:
	RoughConductorMaterial(const std::shared_ptr<FloatSpectralNode>& eta, const std::shared_ptr<FloatSpectralNode>& k,
						   const std::shared_ptr<FloatSpectralNode>& spec,
						   const std::shared_ptr<FloatScalarNode>& roughnessX,
						   const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial()
		, mEta(eta)
		, mK(k)
		, mSpecularity(spec)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
		, mNodeContribFlags(mEta->materialFlags() | mK->materialFlags() | mRoughnessX->flags() | mRoughnessY->flags())
	{
	}

	virtual ~RoughConductorMaterial() = default;

	inline RoughDistribution<IsAnisotropic, UseVNDF> roughness(const ShadingContext& sctx) const
	{
		return RoughDistribution<IsAnisotropic, UseVNDF>(mRoughnessX->eval(sctx), mRoughnessY->eval(sctx));
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.Type = MaterialScatteringType::SpecularReflection;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		if (closure.isDelta()) { // Reject
			out.PDF_S  = 0.0f;
			out.Weight = SpectralBlob::Zero();
			out.Flags  = MaterialSampleFlag::DeltaDistribution | mNodeContribFlags;
			return;
		}

		const SpectralBlob eta = mEta->eval(in.ShadingContext);
		const SpectralBlob k   = mK->eval(in.ShadingContext);
		SpectralBlob factor;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			factor[i] = closure.evalConductor(in.Context.L, in.Context.V, eta[i], k[i]);

		out.Weight = mSpecularity->eval(in.ShadingContext) * factor;
		out.PDF_S  = closure.pdf(in.Context.L, in.Context.V);
		out.Flags  = mNodeContribFlags;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		if (closure.isDelta()) {
			out.PDF_S = 0.0f;
			out.Flags = MaterialSampleFlag::DeltaDistribution | mNodeContribFlags;
			return;
		}

		out.PDF_S = closure.pdf(in.Context.L, in.Context.V);
		out.Flags = mNodeContribFlags;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		out.L			   = closure.sample(in.RND.get2D(), in.Context.V);

		// Set flags
		out.Flags = mNodeContribFlags;
		if (closure.isDelta())
			out.Flags |= MaterialSampleFlag::DeltaDistribution;

		if (!in.Context.V.sameHemisphere(out.L)) { // No transmission
			out = MaterialSampleOutput::Reject(MaterialScatteringType::SpecularReflection, out.Flags);
			return;
		}

		const SpectralBlob eta = mEta->eval(in.ShadingContext);
		const SpectralBlob k   = mK->eval(in.ShadingContext);
		SpectralBlob factor;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			factor[i] = closure.evalConductor(out.L, in.Context.V, eta[i], k[i]);

		out.IntegralWeight = mSpecularity->eval(in.ShadingContext) * factor;
		out.Type		   = MaterialScatteringType::SpecularReflection;
		out.PDF_S		   = closure.pdf(out.L, in.Context.V);

		if (out.PDF_S[0] > PR_EPSILON)
			out.IntegralWeight /= out.PDF_S[0];

		// If we handle a delta case, make sure the outgoing pdf will be 1
		if (closure.isDelta())
			out.PDF_S = 1.0f;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughConductorMaterial>:" << std::endl
			   << "    Eta:             " << mEta->dumpInformation() << std::endl
			   << "    K:               " << mK->dumpInformation() << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl
			   << "    RoughnessX:      " << mRoughnessX->dumpInformation() << std::endl
			   << "    RoughnessY:      " << mRoughnessY->dumpInformation() << std::endl
			   << "    VNDF:            " << (UseVNDF ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;

	const MaterialSampleFlags mNodeContribFlags; // Flags which affect the sampling process
};

// System of function which probably could be simplified with template meta programming
template <bool UseVNDF>
static std::shared_ptr<IMaterial> createMaterial1(const SceneLoadContext& ctx)
{
	std::shared_ptr<FloatScalarNode> rx;
	std::shared_ptr<FloatScalarNode> ry;

	if (ctx.parameters().hasParameter("roughness_x"))
		rx = ctx.lookupScalarNode("roughness_x", 0);
	else
		rx = ctx.lookupScalarNode("roughness", 0);

	if (ctx.parameters().hasParameter("roughness_y"))
		ry = ctx.lookupScalarNode("roughness_y", 0);
	else
		ry = rx;

	const auto eta	= ctx.lookupSpectralNode({ "eta", "index", "ior" }, 1.2f);
	const auto k	= ctx.lookupSpectralNode({ "k", "kappa" }, 2.605f);
	const auto spec = ctx.lookupSpectralNode("specularity", 1);

	if (rx == ry)
		return std::make_shared<RoughConductorMaterial<UseVNDF, false>>(eta, k, spec, rx, ry);
	else
		return std::make_shared<RoughConductorMaterial<UseVNDF, true>>(eta, k, spec, rx, ry);
}

static std::shared_ptr<IMaterial> createMaterial2(const SceneLoadContext& ctx)
{
	const bool use_vndf = ctx.parameters().getBool("vndf", true);

	if (use_vndf)
		return createMaterial1<true>(ctx);
	else
		return createMaterial1<false>(ctx);
}

class RoughConductorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return createMaterial2(ctx);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "roughconductor", "roughmirror", "roughmetal" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Rough Conductor BSDF", "A rough conductor BSDF")
			.Identifiers(getNames())
			.Inputs()
			.BeginBlock("Roughness", PluginParamDescBlockOp::OneOf)
			.ScalarNode("roughness", "Isotropic roughness", 0.0f)
			.BeginBlock("")
			.ScalarNode("roughness_x", "Anisotropic x roughness", 0.0f)
			.ScalarNode("roughness_y", "Anisotropic y roughness", 0.0f)
			.EndBlock()
			.EndBlock()
			.SpectralNodeV({ "index", "eta", "ior" }, "Index of refraction", 1.2f)
			.SpectralNodeV({ "k", "kappa" }, "Absorption index of conductor", 2.605f)
			.SpectralNode("specularity", "Tint", 1.0f)
			.Bool("vndf", "Use sampling method based on the viewing normal", true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RoughConductorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)