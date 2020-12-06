#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Scattering.h"
#include "math/Spherical.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

/* Based on the paper:
	BURLEY, Brent; STUDIOS, Walt Disney Animation. Physically-based shading at disney. In: ACM SIGGRAPH. 2012. S. 1-7.
	and code:
	https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
*/

// If UseVNDF is true, Visible Normals sampling will be used

template <bool UseVNDF>
class PrincipledMaterial : public IMaterial {
public:
	PrincipledMaterial(const std::shared_ptr<FloatSpectralNode>& baseColor,
					   const std::shared_ptr<FloatScalarNode>& spec,
					   const std::shared_ptr<FloatScalarNode>& specTint,
					   const std::shared_ptr<FloatScalarNode>& roughness,
					   const std::shared_ptr<FloatScalarNode>& anisotropic,
					   const std::shared_ptr<FloatScalarNode>& subsurface,
					   const std::shared_ptr<FloatScalarNode>& metallic,
					   const std::shared_ptr<FloatScalarNode>& sheen,
					   const std::shared_ptr<FloatScalarNode>& sheenTint,
					   const std::shared_ptr<FloatScalarNode>& clearcoat,
					   const std::shared_ptr<FloatScalarNode>& clearcoatGloss)
		: IMaterial()
		, mBaseColor(baseColor)
		, mSpecular(spec)
		, mSpecularTint(specTint)
		, mRoughness(roughness)
		, mAnisotropic(anisotropic)
		, mSubsurface(subsurface)
		, mMetallic(metallic)
		, mSheen(sheen)
		, mSheenTint(sheenTint)
		, mClearcoat(clearcoat)
		, mClearcoatGloss(clearcoatGloss)
	{
	}

	virtual ~PrincipledMaterial() = default;

	template <typename T>
	static inline T mix(const T& v0, const T& v1, float t)
	{
		return (1 - t) * v0 + t * v1;
	}

	inline float diffuseTerm(const MaterialEvalContext& ctx, float HdotL, float roughness) const
	{
		float fd90 = 0.5f + 2 * HdotL * HdotL * roughness;
		float lk   = Fresnel::schlick_term(ctx.NdotL());
		float vk   = Fresnel::schlick_term(ctx.NdotV());

		return mix(1.0f, fd90, lk) * mix(1.0f, fd90, vk);
	}

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	inline float subsurfaceTerm(const MaterialEvalContext& ctx, float HdotL, float roughness) const
	{
		float fss90 = HdotL * HdotL * roughness;
		float lk	= Fresnel::schlick_term(ctx.NdotL());
		float vk	= Fresnel::schlick_term(ctx.NdotV());

		float fss = mix(1.0f, fss90, lk) * mix(1.0f, fss90, vk);

		const float f = ctx.NdotL() + ctx.NdotV();
		if (std::abs(f) < PR_EPSILON)
			return 0.0f;
		else
			return 1.25f * (fss * (1.0f / f - 0.5f) + 0.5f);
	}

	inline SpectralBlob specularTerm(const MaterialEvalContext& ctx, const ShadingVector& H, const SpectralBlob& spec,
									 float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		float D		   = Microfacet::ndf_ggx(H, ax, ay);
		float hk	   = Fresnel::schlick_term(H.dot(ctx.L));
		SpectralBlob F = mix<SpectralBlob>(spec, SpectralBlob::Ones(), hk);
		float G		   = Microfacet::g_1_smith_opt(ctx.NdotL(), ctx.XdotL(), ctx.YdotL(), ax, ay)
				  * Microfacet::g_1_smith_opt(ctx.NdotV(), ctx.XdotV(), ctx.YdotV(), ax, ay);

		// 1/(4*NdotV*NdotL) already multiplied out
		return D * F * G;
	}

	inline float clearcoatTerm(const MaterialEvalContext& ctx, const ShadingVector& H, float gloss) const
	{
		// This is fixed by definition
		static float F0 = 0.04f; // IOR 1.5
		static float R	= 0.25f;

		float D	 = Microfacet::ndf_ggx(H.cosTheta(), mix(0.1f, 0.001f, gloss));
		float hk = Fresnel::schlick_term(H.dot(ctx.L));
		float F	 = mix(F0, 1.0f, hk);
		float G	 = Microfacet::g_1_smith_opt(ctx.NdotL(), R) * Microfacet::g_1_smith_opt(ctx.NdotV(), R);

		// 1/(4*NdotV*NdotL) already multiplied out
		return R * D * F * G;
	}

	inline SpectralBlob sheenTerm(const SpectralBlob& sheen, float HdotL) const
	{
		float hk = Fresnel::schlick_term(HdotL);
		return hk * sheen;
	}

	SpectralBlob evalBRDF(const MaterialEvalContext& ctx,
						  const ShadingVector& H,
						  const SpectralBlob& base,
						  float lum,
						  float subsurface,
						  float anisotropic,
						  float roughness,
						  float metallic,
						  float spec,
						  float specTint,
						  float sheen,
						  float sheenTint,
						  float clearcoat,
						  float clearcoatGloss) const
	{
		const SpectralBlob tint = lum > PR_EPSILON
									  ? SpectralBlob(base / lum)
									  : SpectralBlob::Zero();
		const SpectralBlob cspec = mix<SpectralBlob>(
			spec * 0.08f * mix<SpectralBlob>(SpectralBlob::Ones(), tint, specTint),
			base,
			metallic);
		const SpectralBlob csheen = mix<SpectralBlob>(SpectralBlob::Ones(), tint, sheenTint);

		const float HdotL	  = H.dot(ctx.L);
		float diffTerm		  = diffuseTerm(ctx, HdotL, roughness);
		float ssTerm		  = subsurface > PR_EPSILON ? subsurfaceTerm(ctx, HdotL, roughness) : 0.0f;
		SpectralBlob specTerm = specularTerm(ctx, H, cspec, roughness, anisotropic);
		float ccTerm		  = clearcoat > PR_EPSILON ? clearcoatTerm(ctx, H, clearcoatGloss) : 0.0f;
		SpectralBlob shTerm	  = sheenTerm(sheen * csheen, HdotL);

		return (PR_INV_PI * mix(diffTerm, ssTerm, subsurface) * base + shTerm) * (1 - metallic)
			   + specTerm
			   + SpectralBlob(ccTerm) * clearcoat;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.Weight = SpectralBlob::Zero();
			out.PDF_S  = 0;
			out.Type   = MST_SpecularReflection;
			return;
		}

		const auto& sctx	 = in.ShadingContext;
		SpectralBlob base	 = mBaseColor->eval(sctx);
		float lum			 = base.maxCoeff();
		float subsurface	 = mSubsurface->eval(sctx);
		float anisotropic	 = mAnisotropic->eval(sctx);
		float roughness		 = std::max(0.01f, mRoughness->eval(sctx));
		float metallic		 = mMetallic->eval(sctx);
		float spec			 = mSpecular->eval(sctx);
		float specTint		 = mSpecularTint->eval(sctx);
		float sheen			 = mSheen->eval(sctx);
		float sheenTint		 = mSheenTint->eval(sctx);
		float clearcoat		 = mClearcoat->eval(sctx);
		float clearcoatGloss = mClearcoatGloss->eval(sctx);

		const Vector3f H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		out.Weight		 = evalBRDF(in.Context, H,
								base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
								sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(in.Context.NdotL());

		float aspect = std::sqrt(1 - anisotropic * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);
		if constexpr (UseVNDF)
			out.PDF_S = Microfacet::pdf_ggx_vndf(in.Context.V, H, ax, ay);
		else
			out.PDF_S = Microfacet::pdf_ggx(H, ax, ay);
		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");

		if (roughness < 0.5f)
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		if (!in.Context.V.sameHemisphere(in.Context.L)) {
			out.PDF_S = 0;
			return;
		}

		const auto& sctx  = in.ShadingContext;
		float anisotropic = mAnisotropic->eval(sctx);
		float roughness	  = std::max(0.01f, mRoughness->eval(sctx));

		const Vector3f H = Scattering::halfway_reflection(in.Context.V, in.Context.L);

		float aspect = std::sqrt(1 - anisotropic * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);
		if constexpr (UseVNDF)
			out.PDF_S = Microfacet::pdf_ggx_vndf(in.Context.V, H, ax, ay);
		else
			out.PDF_S = Microfacet::pdf_ggx(H, ax, ay);
		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");
	}

	void sampleSpecularPath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out, float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		if constexpr (UseVNDF) {
			out.L	  = Microfacet::sample_vndf_ggx(u, v, in.Context.V, ax, ay);
			out.PDF_S = Microfacet::pdf_ggx_vndf(in.Context.V, out.L, ax, ay);
		} else {
			out.L	  = Microfacet::sample_ndf_ggx(u, v, ax, ay);
			out.PDF_S = Microfacet::pdf_ggx(out.L, ax, ay);
		}

		const float refInvPdf = 2 * std::max(0.0f, out.L.dot(in.Context.V));
		if (refInvPdf > PR_EPSILON && out.L[2] > PR_EPSILON)
			out.PDF_S /= refInvPdf;
		else // Drop sample
			out.PDF_S = 0.0f;

		// Reflect incoming ray by the calculated half vector
		out.L = Scattering::reflect(in.Context.V, out.L).normalized();
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx	 = in.ShadingContext;
		SpectralBlob base	 = mBaseColor->eval(sctx);
		float lum			 = base.maxCoeff();
		float subsurface	 = mSubsurface->eval(sctx);
		float anisotropic	 = mAnisotropic->eval(sctx);
		float roughness		 = mRoughness->eval(sctx);
		float metallic		 = mMetallic->eval(sctx);
		float spec			 = mSpecular->eval(sctx);
		float specTint		 = mSpecularTint->eval(sctx);
		float sheen			 = mSheen->eval(sctx);
		float sheenTint		 = mSheenTint->eval(sctx);
		float clearcoat		 = mClearcoat->eval(sctx);
		float clearcoatGloss = mClearcoatGloss->eval(sctx);

		sampleSpecularPath(in, in.RND[0] /* / weight*/, in.RND[1], out, roughness, anisotropic);
		if (!in.Context.V.sameHemisphere(out.L)) { // Side check
			out = MaterialSampleOutput::Reject(MST_SpecularReflection);
			return;
		}

		if (roughness < in.RND[0])
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;

		out.Weight = evalBRDF(in.Context.expand(out.L),
							  Scattering::halfway_reflection(in.Context.V, out.L),
							  base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * out.L.absCosTheta();

		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <PrincipledMaterial>:" << std::endl
			   << "    BaseColor:      " << mBaseColor->dumpInformation() << std::endl
			   << "    Specular:       " << mSpecular->dumpInformation() << std::endl
			   << "    SpecularTint:   " << mSpecularTint->dumpInformation() << std::endl
			   << "    Roughness:      " << mRoughness->dumpInformation() << std::endl
			   << "    Anisotropic:    " << mAnisotropic->dumpInformation() << std::endl
			   << "    Subsurface:     " << mSubsurface->dumpInformation() << std::endl
			   << "    Metallic:       " << mMetallic->dumpInformation() << std::endl
			   << "    Sheen:          " << mSheen->dumpInformation() << std::endl
			   << "    SheenTint:      " << mSheenTint->dumpInformation() << std::endl
			   << "    Clearcoat:      " << mClearcoat->dumpInformation() << std::endl
			   << "    ClearcoatGloss: " << mClearcoatGloss->dumpInformation() << std::endl
			   << "    VNDF:           " << (UseVNDF ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mBaseColor;
	std::shared_ptr<FloatScalarNode> mSpecular;
	std::shared_ptr<FloatScalarNode> mSpecularTint;
	std::shared_ptr<FloatScalarNode> mRoughness;
	std::shared_ptr<FloatScalarNode> mAnisotropic;
	std::shared_ptr<FloatScalarNode> mSubsurface;
	std::shared_ptr<FloatScalarNode> mMetallic;
	std::shared_ptr<FloatScalarNode> mSheen;
	std::shared_ptr<FloatScalarNode> mSheenTint;
	std::shared_ptr<FloatScalarNode> mClearcoat;
	std::shared_ptr<FloatScalarNode> mClearcoatGloss;
}; // namespace PR

template <bool UseVNDF>
inline static std::shared_ptr<IMaterial> createPrincipled(const SceneLoadContext& ctx)
{
	const auto base_color	   = ctx.lookupSpectralNode({ "base_color", "base" }, 0.8f);
	const auto specular		   = ctx.lookupScalarNode("specular", 0.5f);
	const auto specular_tint   = ctx.lookupScalarNode("specular_tint", 0.0f);
	const auto roughness	   = ctx.lookupScalarNode("roughness", 0.5f);
	const auto anisotropic	   = ctx.lookupScalarNode("anisotropic", 0.0f);
	const auto subsurface	   = ctx.lookupScalarNode("subsurface", 0.0f);
	const auto metallic		   = ctx.lookupScalarNode("metallic", 0.0f);
	const auto sheen		   = ctx.lookupScalarNode("sheen", 0.0f);
	const auto sheen_tint	   = ctx.lookupScalarNode("sheen_tint", 0.0f);
	const auto clearcoat	   = ctx.lookupScalarNode("clearcoat", 0.0f);
	const auto clearcoat_gloss = ctx.lookupScalarNode("clearcoat_gloss", 0.0f);

	return std::make_shared<PrincipledMaterial<UseVNDF>>(
		base_color, specular, specular_tint, roughness,
		anisotropic, subsurface, metallic, sheen,
		sheen_tint, clearcoat, clearcoat_gloss);
}

class PrincipledMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		// TODO: Fix VNDF and make it default again
		const bool use_vndf = ctx.parameters().getBool("vndf", false);

		if (use_vndf)
			return createPrincipled<true>(ctx);
		else
			return createPrincipled<false>(ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "principled" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PrincipledMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)