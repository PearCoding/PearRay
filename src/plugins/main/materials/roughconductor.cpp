#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Scattering.h"

#include "renderer/RenderContext.h"

#include "Roughness.h"

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
		, mRoughness(roughnessX, roughnessY)
	{
	}

	virtual ~RoughConductorMaterial() = default;

	inline SpectralBlob fresnelTerm(float dot, const ShadingContext& sctx) const
	{
		const SpectralBlob eta = mEta->eval(sctx);
		const SpectralBlob k   = mK->eval(sctx);

		SpectralBlob fresnel;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			fresnel[i] = Fresnel::conductor(dot, eta[i], k[i]);

		return fresnel;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.Type = MST_SpecularReflection;

		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.PDF_S  = 0.0f;
			out.Weight = SpectralBlob::Zero();
			return;
		}

		const ShadingVector H	   = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		const float HdotV		   = H.dot(in.Context.V);
		const SpectralBlob fresnel = fresnelTerm(in.Context.V.absCosTheta(), in.ShadingContext);
		const auto closure		   = mRoughness.closure(in.ShadingContext);

		const float DGNorm	 = closure.DGNorm(H, in.Context.V, in.Context.L);
		const float jacobian = Scattering::reflective_jacobian(HdotV);
		const float pdf		 = closure.pdf(H, in.Context.V);

		out.Weight = fresnel * mSpecularity->eval(in.ShadingContext) * (DGNorm * jacobian);
		out.PDF_S  = pdf * jacobian;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.PDF_S = 0;
			return;
		}

		const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		const float HdotV	  = H.dot(in.Context.V);
		const auto closure	  = mRoughness.closure(in.ShadingContext);
		const float jacobian  = Scattering::reflective_jacobian(HdotV);
		const float pdf		  = closure.pdf(H, in.Context.V);
		out.PDF_S			  = pdf * jacobian;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Sample microfacet normal
		const auto closure = mRoughness.closure(in.ShadingContext);
		const Vector3f H   = closure.sample(in.RND, in.Context.V);

		// Set flags
		if constexpr (SpectralVarying)
			out.Flags = (closure.isDelta() ? MSF_DeltaDistribution : 0) | MSF_SpectralVarying;
		else
			out.Flags = (closure.isDelta() ? MSF_DeltaDistribution : 0);

		const float HdotV = std::abs(H.dot((Vector3f)in.Context.V));
		if (HdotV <= PR_EPSILON) {
			out = MaterialSampleOutput::Reject(MST_SpecularReflection, out.Flags);
			return;
		}

		// Calculate Fresnel term
		out.L = Scattering::reflect(in.Context.V, H);

		if (!in.Context.V.sameHemisphere(out.L)) { // Side check
			out = MaterialSampleOutput::Reject(MST_SpecularReflection, out.Flags);
			return;
		}

		const SpectralBlob fresnel = fresnelTerm(in.Context.V.absCosTheta(), in.ShadingContext);
		out.Weight				   = fresnel * mSpecularity->eval(in.ShadingContext);
		out.Type				   = MST_SpecularReflection;
		out.PDF_S				   = closure.pdf(H, in.Context.V);

		if (!closure.isDelta()) {
			const float DGNorm	 = closure.DGNorm(H, in.Context.V, out.L);
			const float jacobian = Scattering::reflective_jacobian(HdotV);
			out.Weight *= DGNorm * jacobian;
			out.PDF_S *= jacobian;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughConductorMaterial>:" << std::endl
			   << "    Eta:             " << mEta->dumpInformation() << std::endl
			   << "    K:               " << mK->dumpInformation() << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl
			   << "    RoughnessX:      " << mRoughness.roughnessX()->dumpInformation() << std::endl
			   << "    RoughnessY:      " << mRoughness.roughnessY()->dumpInformation() << std::endl
			   << "    SpectralVarying: " << (SpectralVarying ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mEta;
	const std::shared_ptr<FloatSpectralNode> mK;
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const Roughness<IsAnisotropic, UseVNDF, false> mRoughness;
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
	// TODO: Fix VNDF and make it default again
	const bool use_vndf = ctx.parameters().getBool("vndf", false);

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