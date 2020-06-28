#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Spherical.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

/* Based on the paper:
	BURLEY, Brent; STUDIOS, Walt Disney Animation. Physically-based shading at disney. In: ACM SIGGRAPH. 2012. S. 1-7.
	and code:
	https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
*/

class PrincipledMaterial : public IMaterial {
public:
	PrincipledMaterial(uint32 id,
					   const std::shared_ptr<FloatSpectralNode>& baseColor,
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
		: IMaterial(id)
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

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

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

	inline SpectralBlob specularTerm(const MaterialEvalContext& ctx, const SpectralBlob& spec,
									 float HdotL, float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		float D		   = Microfacet::ndf_ggx(ctx.NdotH(), ctx.XdotH(), ctx.YdotH(), ax, ay);
		float hk	   = Fresnel::schlick_term(HdotL);
		SpectralBlob F = mix<SpectralBlob>(spec, SpectralBlob::Ones(), hk);
		float G		   = Microfacet::g_1_smith(ctx.NdotL(), ctx.XdotL(), ctx.YdotL(), ax, ay)
				  * Microfacet::g_1_smith(ctx.NdotV(), ctx.XdotV(), ctx.YdotV(), ax, ay);

		// 1/(4*NdotV*NdotL) already multiplied out
		return D * F * G;
	}

	inline float clearcoatTerm(const MaterialEvalContext& ctx, float gloss, float HdotL) const
	{
		// This is fixed by definition
		static float F0 = 0.04f; // IOR 1.5
		static float R	= 0.25f;

		float D	 = Microfacet::ndf_ggx(ctx.NdotH(), mix(0.1f, 0.001f, gloss));
		float hk = Fresnel::schlick_term(HdotL);
		float F	 = mix(F0, 1.0f, hk);
		float G	 = Microfacet::g_1_smith(ctx.NdotL(), R) * Microfacet::g_1_smith(ctx.NdotV(), R);

		// 1/(4*NdotV*NdotL) already multiplied out
		return R * D * F * G;
	}

	inline SpectralBlob sheenTerm(const SpectralBlob& sheen, float HdotL) const
	{
		float hk = Fresnel::schlick_term(HdotL);
		return hk * sheen;
	}

	SpectralBlob evalBRDF(const MaterialEvalContext& ctx,
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
		const float HdotL = ctx.L.dot(ctx.H);

		const SpectralBlob tint = lum > PR_EPSILON
									  ? SpectralBlob(base / lum)
									  : SpectralBlob::Zero();
		const SpectralBlob cspec = mix<SpectralBlob>(
			spec * 0.08f * mix<SpectralBlob>(SpectralBlob::Ones(), tint, specTint),
			base,
			metallic);
		const SpectralBlob csheen = mix<SpectralBlob>(SpectralBlob::Ones(), tint, sheenTint);

		float diffTerm		  = diffuseTerm(ctx, HdotL, roughness);
		float ssTerm		  = subsurface > PR_EPSILON ? subsurfaceTerm(ctx, HdotL, roughness) : 0.0f;
		SpectralBlob specTerm = specularTerm(ctx, cspec, HdotL, roughness, anisotropic);
		float ccTerm		  = clearcoat > PR_EPSILON ? clearcoatTerm(ctx, clearcoatGloss, HdotL) : 0.0f;
		SpectralBlob shTerm	  = sheenTerm(sheen * csheen, HdotL);

		return (PR_1_PI * mix(diffTerm, ssTerm, subsurface) * base + shTerm) * (1 - metallic)
			   + specTerm
			   + SpectralBlob(ccTerm) * clearcoat;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

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

		out.Weight = evalBRDF(in.Context, base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(in.Context.NdotL());

		float aspect = std::sqrt(1 - anisotropic * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);
		out.PDF_S	 = Microfacet::pdf_ggx(in.Context.NdotL(), ax, ay); // Use NdotH instead of NdotL!

		if (roughness < 0.5f)
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out, float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		float pdf_s;
		out.L = Microfacet::sample_ggx_vndf(u, v, in.Context.V, ax, ay, pdf_s);
		//out.Outgoing = Microfacet::sample_ndf_ggx(u, v, ax, ay, pdf_s);
		out.PDF_S = pdf_s;

		const float refInvPdf = 2 * std::max(0.0f, out.L.dot(in.Context.V));
		if (refInvPdf > PR_EPSILON)
			out.PDF_S /= refInvPdf;
		else
			out.PDF_S = 0.0f; // Drop sample

		if (out.L[2] <= PR_EPSILON)
			out.PDF_S = 0;

		// Reflect incoming ray by the calculated half vector
		out.L = Reflection::reflect(in.Context.V, out.L);
		out.L.normalize();

		if (out.L[2] <= PR_EPSILON)
			out.PDF_S = 0;
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

		if (roughness < in.RND[0])
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;

		out.Weight = evalBRDF(in.Context.expand(out.L),
							  base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(out.L[2]);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <PrincipledMaterial>:" << std::endl
			   << "    BaseColor:      " << (mBaseColor ? mBaseColor->dumpInformation() : "NONE") << std::endl
			   << "    Specular:       " << (mSpecular ? mSpecular->dumpInformation() : "NONE") << std::endl
			   << "    SpecularTint:   " << (mSpecularTint ? mSpecularTint->dumpInformation() : "NONE") << std::endl
			   << "    Roughness:      " << (mRoughness ? mRoughness->dumpInformation() : "NONE") << std::endl
			   << "    Anisotropic:    " << (mAnisotropic ? mAnisotropic->dumpInformation() : "NONE") << std::endl
			   << "    Subsurface:     " << (mSubsurface ? mSubsurface->dumpInformation() : "NONE") << std::endl
			   << "    Metallic:       " << (mMetallic ? mMetallic->dumpInformation() : "NONE") << std::endl
			   << "    Sheen:          " << (mSheen ? mSheen->dumpInformation() : "NONE") << std::endl
			   << "    SheenTint:      " << (mSheenTint ? mSheenTint->dumpInformation() : "NONE") << std::endl
			   << "    Clearcoat:      " << (mClearcoat ? mClearcoat->dumpInformation() : "NONE") << std::endl
			   << "    ClearcoatGloss: " << (mClearcoatGloss ? mClearcoatGloss->dumpInformation() : "NONE") << std::endl;

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

class PrincipledMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<PrincipledMaterial>(id,
													ctx.Env->lookupSpectralNode(params.getParameter("base_color"), 0.8f),
													ctx.Env->lookupScalarNode(params.getParameter("specular"), 0.5f),
													ctx.Env->lookupScalarNode(params.getParameter("specular_tint"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("roughness"), 0.5f),
													ctx.Env->lookupScalarNode(params.getParameter("anisotropic"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("subsurface"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("metallic"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("sheen"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("sheen_tint"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("clearcoat"), 0.0f),
													ctx.Env->lookupScalarNode(params.getParameter("clearcoat_gloss"), 0.0f));
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