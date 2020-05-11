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
					   const std::shared_ptr<FloatSpectralShadingSocket>& baseColor,
					   const std::shared_ptr<FloatScalarShadingSocket>& spec,
					   const std::shared_ptr<FloatScalarShadingSocket>& specTint,
					   const std::shared_ptr<FloatScalarShadingSocket>& roughness,
					   const std::shared_ptr<FloatScalarShadingSocket>& anisotropic,
					   const std::shared_ptr<FloatScalarShadingSocket>& subsurface,
					   const std::shared_ptr<FloatScalarShadingSocket>& metallic,
					   const std::shared_ptr<FloatScalarShadingSocket>& sheen,
					   const std::shared_ptr<FloatScalarShadingSocket>& sheenTint,
					   const std::shared_ptr<FloatScalarShadingSocket>& clearcoat,
					   const std::shared_ptr<FloatScalarShadingSocket>& clearcoatGloss)
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

	inline float diffuseTerm(const ShadingPoint& point, float NdotL, float HdotL, float roughness) const
	{
		float fd90 = 0.5f + 2 * HdotL * HdotL * roughness;
		float lk   = Fresnel::schlick_term(NdotL);
		float vk   = Fresnel::schlick_term(-point.NdotV);

		return mix(1.0f, fd90, lk) * mix(1.0f, fd90, vk);
	}

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	inline float subsurfaceTerm(const ShadingPoint& point, float NdotL, float HdotL, float roughness) const
	{
		float fss90 = HdotL * HdotL * roughness;
		float lk	= Fresnel::schlick_term(NdotL);
		float vk	= Fresnel::schlick_term(-point.NdotV);

		float fss = mix(1.0f, fss90, lk) * mix(1.0f, fss90, vk);

		const float f = NdotL - point.NdotV;
		if (std::abs(f) < PR_EPSILON)
			return 0.0f;
		else
			return 1.25f * (fss * (1.0f / f - 0.5f) + 0.5f);
	}

	inline SpectralBlob specularTerm(const ShadingPoint& point, const SpectralBlob& spec,
									 float VdotX, float VdotY,
									 float NdotL, float LdotX, float LdotY,
									 float HdotL, float NdotH, float HdotX, float HdotY,
									 float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		float D		   = Microfacet::ndf_ggx(NdotH, HdotX, HdotY, ax, ay);
		float hk	   = Fresnel::schlick_term(HdotL);
		SpectralBlob F = mix<SpectralBlob>(spec, SpectralBlob::Ones(), hk);
		float G		   = Microfacet::g_1_smith(NdotL, LdotX, LdotY, ax, ay) * Microfacet::g_1_smith(-point.NdotV, VdotX, VdotY, ax, ay);

		// 1/(4*NdotV*NdotL) already multiplied out
		return D * F * G;
	}

	inline float clearcoatTerm(const ShadingPoint& point, float gloss, float NdotL, float HdotL, float NdotH) const
	{
		// This is fixed by definition
		static float F0 = 0.04f; // IOR 1.5
		static float R  = 0.25f;

		float D  = Microfacet::ndf_ggx(NdotH, mix(0.1f, 0.001f, gloss));
		float hk = Fresnel::schlick_term(HdotL);
		float F  = mix(F0, 1.0f, hk);
		float G  = Microfacet::g_1_smith(NdotL, R) * Microfacet::g_1_smith(-point.NdotV, R);

		// 1/(4*NdotV*NdotL) already multiplied out
		return R * D * F * G;
	}

	inline SpectralBlob sheenTerm(const SpectralBlob& sheen, float HdotL) const
	{
		float hk = Fresnel::schlick_term(HdotL);
		return hk * sheen;
	}

	SpectralBlob evalBRDF(const ShadingPoint& point, const Vector3f& L,
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
		const float VdotX = -point.Nx.dot(point.Ray.Direction);
		const float VdotY = -point.Ny.dot(point.Ray.Direction);
		const float NdotL = point.N.dot(L);
		const float LdotX = point.Nx.dot(L);
		const float LdotY = point.Ny.dot(L);

		const Eigen::Vector3f H = Reflection::halfway(point.Ray.Direction, L);
		const float NdotH		= point.N.dot(H);
		const float HdotL		= L.dot(H);
		const float HdotX		= point.Nx.dot(H);
		const float HdotY		= point.Ny.dot(H);

		const SpectralBlob tint = lum > PR_EPSILON
									  ? SpectralBlob(base / lum)
									  : SpectralBlob::Zero();
		const SpectralBlob cspec = mix<SpectralBlob>(
			spec * 0.08f * mix<SpectralBlob>(SpectralBlob::Ones(), tint, specTint),
			base,
			metallic);
		const SpectralBlob csheen = mix<SpectralBlob>(SpectralBlob::Ones(), tint, sheenTint);

		float diffTerm		  = diffuseTerm(point, NdotL, HdotL, roughness);
		float ssTerm		  = subsurface > PR_EPSILON ? subsurfaceTerm(point, NdotL, HdotL, roughness) : 0.0f;
		SpectralBlob specTerm = specularTerm(point, cspec,
											 VdotX, VdotY, NdotL, LdotX, LdotY, HdotL, NdotH, HdotX, HdotY,
											 roughness, anisotropic);
		float ccTerm		  = clearcoat > PR_EPSILON ? clearcoatTerm(point, clearcoatGloss, NdotL, HdotL, NdotH) : 0.0f;
		SpectralBlob shTerm   = sheenTerm(sheen * csheen, HdotL);

		return (PR_1_PI * mix(diffTerm, ssTerm, subsurface) * base + shTerm) * (1 - metallic)
			   + specTerm
			   + SpectralBlob(ccTerm) * clearcoat;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		SpectralBlob base	= mBaseColor->eval(in.Point);
		float lum			 = mBaseColor->relativeLuminance(in.Point);
		float subsurface	 = mSubsurface->eval(in.Point);
		float anisotropic	= mAnisotropic->eval(in.Point);
		float roughness		 = std::max(0.01f, mRoughness->eval(in.Point));
		float metallic		 = mMetallic->eval(in.Point);
		float spec			 = mSpecular->eval(in.Point);
		float specTint		 = mSpecularTint->eval(in.Point);
		float sheen			 = mSheen->eval(in.Point);
		float sheenTint		 = mSheenTint->eval(in.Point);
		float clearcoat		 = mClearcoat->eval(in.Point);
		float clearcoatGloss = mClearcoatGloss->eval(in.Point);

		out.Weight = evalBRDF(in.Point, in.Outgoing,
							  base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(in.NdotL);

		float aspect = std::sqrt(1 - anisotropic * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);
		out.PDF_S	= Microfacet::pdf_ggx(in.NdotL, ax, ay); // Use NdotH instead of NdotL!

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

		Vector3f nV  = Tangent::toTangentSpace<float>(in.Point.N, in.Point.Nx, in.Point.Ny, -in.Point.Ray.Direction);
		out.Outgoing = Microfacet::sample_ggx_vndf(u, v, nV, ax, ay, out.PDF_S);
		//out.Outgoing = Microfacet::sample_ndf_ggx(u, v, ax, ay, out.PDF_S);

		const float refInvPdf = 2 * std::max(0.0f, out.Outgoing.dot(nV));
		if (refInvPdf > PR_EPSILON)
			out.PDF_S /= refInvPdf;
		else
			out.PDF_S = 0.0f; // Drop sample

		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		if (out.Outgoing.dot(in.Point.N) <= PR_EPSILON)
			out.PDF_S = 0;

		// Reflect incoming ray by the calculated half vector
		out.Outgoing = Reflection::reflect(out.Outgoing.dot(in.Point.Ray.Direction), out.Outgoing, in.Point.Ray.Direction);
		out.Outgoing.normalize();

		if (out.Outgoing.dot(in.Point.N) <= PR_EPSILON)
			out.PDF_S = 0;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		SpectralBlob base	= mBaseColor->eval(in.Point);
		float lum			 = mBaseColor->relativeLuminance(in.Point);
		float subsurface	 = mSubsurface->eval(in.Point);
		float anisotropic	= mAnisotropic->eval(in.Point);
		float roughness		 = mRoughness->eval(in.Point);
		float metallic		 = mMetallic->eval(in.Point);
		float spec			 = mSpecular->eval(in.Point);
		float specTint		 = mSpecularTint->eval(in.Point);
		float sheen			 = mSheen->eval(in.Point);
		float sheenTint		 = mSheenTint->eval(in.Point);
		float clearcoat		 = mClearcoat->eval(in.Point);
		float clearcoatGloss = mClearcoatGloss->eval(in.Point);

		sampleSpecularPath(in, in.RND[0] /* / weight*/, in.RND[1], out, roughness, anisotropic);

		if (roughness < in.RND[0])
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;

		out.Weight = evalBRDF(in.Point, out.Outgoing,
							  base, lum, subsurface, anisotropic, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(in.Point.N.dot(out.Outgoing));
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
	std::shared_ptr<FloatSpectralShadingSocket> mBaseColor;
	std::shared_ptr<FloatScalarShadingSocket> mSpecular;
	std::shared_ptr<FloatScalarShadingSocket> mSpecularTint;
	std::shared_ptr<FloatScalarShadingSocket> mRoughness;
	std::shared_ptr<FloatScalarShadingSocket> mAnisotropic;
	std::shared_ptr<FloatScalarShadingSocket> mSubsurface;
	std::shared_ptr<FloatScalarShadingSocket> mMetallic;
	std::shared_ptr<FloatScalarShadingSocket> mSheen;
	std::shared_ptr<FloatScalarShadingSocket> mSheenTint;
	std::shared_ptr<FloatScalarShadingSocket> mClearcoat;
	std::shared_ptr<FloatScalarShadingSocket> mClearcoatGloss;
}; // namespace PR

class PrincipledMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;
		return std::make_shared<PrincipledMaterial>(id,
													ctx.Env->lookupSpectralShadingSocket(params.getParameter("base_color"), 0.8f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("specular"), 0.5f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("specular_tint"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("roughness"), 0.5f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("anisotropic"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("subsurface"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("metallic"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("sheen"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("sheen_tint"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("clearcoat"), 0.0f),
													ctx.Env->lookupScalarShadingSocket(params.getParameter("clearcoat_gloss"), 0.0f));
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