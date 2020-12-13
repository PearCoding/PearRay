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
template <bool SpectralVarying, bool UseVNDF, bool IsAnisotropic>
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
		out.Type = MST_SpecularReflection;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		if (closure.isDelta()) { // Reject
			out.PDF_S  = 0.0f;
			out.Weight = SpectralBlob::Zero();
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
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		if (closure.isDelta()) {
			out.PDF_S = 0.0f;
			return;
		}
		out.PDF_S = closure.pdf(in.Context.L, in.Context.V);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = MicrofacetReflection(roughness(in.ShadingContext));
		out.L			   = closure.sample(in.RND.get2D(), in.Context.V);

		// Set flags
		if constexpr (SpectralVarying)
			out.Flags = (closure.isDelta() ? MSF_DeltaDistribution : 0) | MSF_SpectralVarying;
		else
			out.Flags = (closure.isDelta() ? MSF_DeltaDistribution : 0);

		if (!in.Context.V.sameHemisphere(out.L)) { // No transmission
			out = MaterialSampleOutput::Reject(MST_SpecularReflection, out.Flags);
			return;
		}
		
		const SpectralBlob eta = mEta->eval(in.ShadingContext);
		const SpectralBlob k   = mK->eval(in.ShadingContext);
		SpectralBlob factor;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			factor[i] = closure.evalConductor(out.L, in.Context.V, eta[i], k[i]);

		out.Weight = mSpecularity->eval(in.ShadingContext) * factor;
		out.Type   = MST_SpecularReflection;
		out.PDF_S  = closure.pdf(out.L, in.Context.V);
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
			   << "    SpectralVarying: " << (SpectralVarying ? "true" : "false") << std::endl
			   << "    VNDF:            " << (UseVNDF ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;
};

// System of function which probably could be simplified with template meta programming
template <bool SpectralVarying, bool UseVNDF>
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
		return std::make_shared<RoughConductorMaterial<SpectralVarying, UseVNDF, false>>(eta, k, spec, rx, ry);
	else
		return std::make_shared<RoughConductorMaterial<SpectralVarying, UseVNDF, true>>(eta, k, spec, rx, ry);
}

template <bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial2(const SceneLoadContext& ctx)
{
	const bool use_vndf = ctx.parameters().getBool("vndf", true);

	if (use_vndf)
		return createMaterial1<SpectralVarying, true>(ctx);
	else
		return createMaterial1<SpectralVarying, false>(ctx);
}

static std::shared_ptr<IMaterial> createMaterial3(const SceneLoadContext& ctx)
{
	// Contrary to the dielectric interface, conductors are not spectral varying per default
	// as it is quite uncommon to use ior functions
	const bool spectralVarying = ctx.parameters().getBool("spectral_varying", false);
	if (spectralVarying)
		return createMaterial2<true>(ctx);
	else
		return createMaterial2<false>(ctx);
}

class RoughConductorMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		return createMaterial3(ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "roughconductor", "roughmetal" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RoughConductorMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)