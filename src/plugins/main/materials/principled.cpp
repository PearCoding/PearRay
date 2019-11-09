#include "Environment.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Spherical.h"
#include "renderer/RenderContext.h"
#include "shader/ShadingSocket.h"

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

		return 1.25f * (fss * (1.0f / (NdotL - point.NdotV) - 0.5f) + 0.5f);
	}

	inline float specularTerm(const ShadingPoint& point, float spec,
							  float VdotX, float VdotY,
							  float NdotL, float LdotX, float LdotY,
							  float HdotL, float NdotH, float HdotX, float HdotY,
							  float roughness, float aniso) const
	{
		float aspect = aniso > 1.0f ? std::sqrt(1 - aniso * 0.9f) : 1.0f;
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		float D  = Microfacet::ndf_ggx(NdotH, HdotX, HdotY, ax, ay);
		float hk = Fresnel::schlick_term(HdotL);
		float F  = mix(spec, 1.0f, hk);
		float G  = Microfacet::g_1_smith(NdotL, LdotX, LdotY, ax, ay) * Microfacet::g_1_smith(-point.NdotV, VdotX, VdotY, ax, ay);

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

	inline float sheenTerm(float sheen, float HdotL) const
	{
		float hk = Fresnel::schlick_term(HdotL);
		return hk * sheen;
	}

	float evalBRDF(const ShadingPoint& point, const Vector3f& L,
				   float base,
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

		const float tint   = lum > PR_EPSILON ? base / lum : 1;
		const float cspec  = mix(spec * 0.08f * mix(1.0f, tint, specTint), base, metallic);
		const float csheen = mix(1.0f, tint, sheenTint);

		float diffTerm = diffuseTerm(point, NdotL, HdotL, roughness);
		float ssTerm   = subsurface > PR_EPSILON ? subsurfaceTerm(point, NdotL, HdotL, roughness) : 0.0f;
		float specTerm = specularTerm(point, cspec,
									  VdotX, VdotY, NdotL, LdotX, LdotY, HdotL, NdotH, HdotX, HdotY,
									  roughness, anisotropic);
		float ccTerm   = clearcoat > PR_EPSILON ? clearcoatTerm(point, clearcoatGloss, NdotL, HdotL, NdotH) : 0.0f;
		float shTerm   = sheenTerm(sheen * csheen, HdotL);

		return (PR_1_PI * mix(diffTerm, ssTerm, subsurface) * base + shTerm) * (1 - metallic)
			   + specTerm
			   + ccTerm * clearcoat;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		float base			 = mBaseColor->eval(in.Point);
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

		float weight = (1 - roughness) * std::max(0.1f, metallic);
		out.PDF_S	= mix(Projection::cos_hemi_pdf(in.NdotL), Microfacet::pdf_ggx(in.NdotL, roughness), weight); // Use NdotH instead of NdotL!

		if (metallic < 0.5f)
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out) const
	{
		out.Outgoing = Projection::cos_hemi(u, v, out.PDF_S);
		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		out.Type	 = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out, float roughness, float aniso) const
	{
		float aspect = std::sqrt(1 - aniso * 0.9f);
		float ax	 = std::max(0.001f, roughness * roughness / aspect);
		float ay	 = std::max(0.001f, roughness * roughness * aspect);

		Vector3f nV  = Tangent::toTangentSpace<float>(in.Point.N, in.Point.Nx, in.Point.Ny, -in.Point.Ray.Direction);
		out.Outgoing = Microfacet::sample_ggx_vndf(u, v, nV, ax, ay, out.PDF_S);
		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		// Reflect incoming ray by the calculated half vector
		out.Outgoing = Reflection::reflect(out.Outgoing.dot(in.Point.Ray.Direction), out.Outgoing, in.Point.Ray.Direction);
		out.Type	 = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		float base			 = mBaseColor->eval(in.Point);
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

		float weight = (1 - roughness) * std::max(0.1f, metallic);
		if (in.RND[0] < weight) {
			sampleSpecularPath(in, in.RND[0] / weight, in.RND[1], out, roughness, anisotropic);
			out.PDF_S *= weight;
		} else {
			sampleDiffusePath(in, (in.RND[0] - weight) / (1 - weight), in.RND[1], out);
			out.PDF_S *= (1 - weight);
		}

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

protected:
	void onFreeze(RenderContext*) override
	{
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

class PrincipledMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string baseColorName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "base_color", "");
		const std::string specularName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specular", "");
		const std::string specularTintName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specular_tint", "");
		const std::string roughnessName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "roughness", "");
		const std::string anisotropicName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "anisotropic", "");
		const std::string subsurfaceName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "subsurface", "");
		const std::string metallicName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "metallic", "");
		const std::string sheenName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "sheen", "");
		const std::string sheenTintName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "sheen_tint", "");
		const std::string clearcoatName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "clearcoat", "");
		const std::string clearcoatTintName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "clearcoat_tint", "");

		return std::make_shared<PrincipledMaterial>(id,
													env.getSpectralShadingSocket(baseColorName, 1.0f),
													env.getScalarShadingSocket(specularName, 0.5f),
													env.getScalarShadingSocket(specularTintName, 0.0f),
													env.getScalarShadingSocket(roughnessName, 0.5f),
													env.getScalarShadingSocket(anisotropicName, 0.0f),
													env.getScalarShadingSocket(subsurfaceName, 0.0f),
													env.getScalarShadingSocket(metallicName, 0.0f),
													env.getScalarShadingSocket(sheenName, 0.0f),
													env.getScalarShadingSocket(sheenTintName, 0.0f),
													env.getScalarShadingSocket(clearcoatName, 0.0f),
													env.getScalarShadingSocket(clearcoatTintName, 0.0f));
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

PR_PLUGIN_INIT(PR::PrincipledMaterialFactory, "mat_principled", PR_PLUGIN_VERSION)