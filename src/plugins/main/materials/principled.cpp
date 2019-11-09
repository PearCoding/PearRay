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

	inline float diffuseTerm(const ShadingPoint& point, float NdotL, float HdotL, float roughness) const
	{
		float fd90 = 0.5f + 2 * HdotL * HdotL * roughness;
		float lk   = 1 - NdotL;
		lk		   = lk * lk * lk * lk * lk;

		float vk = 1 + point.NdotV;
		vk		 = vk * vk * vk * vk * vk;

		return PR_1_PI * (1 + (fd90 - 1) * lk) * (1 + (fd90 - 1) * vk);
	}

	inline float subsurfaceTerm(const ShadingPoint& point, float NdotL, float HdotL, float roughness) const
	{
		float fss90 = HdotL * HdotL * roughness;
		float lk	= 1 - NdotL;
		lk			= lk * lk * lk * lk * lk;

		float vk = 1 + point.NdotV;
		vk		 = vk * vk * vk * vk * vk;

		float fss = (1 + (fss90 - 1) * lk) * (1 + (fss90 - 1) * vk);

		return PR_1_PI * 1.25f * (fss * (1.0f / (NdotL - point.NdotV) - 0.5f) + 0.5f);
	}

	inline float specularTerm(const ShadingPoint& point, float spec, float NdotL, float HdotL, float NdotH, float roughness) const
	{
		float D  = Microfacet::ndf_ggx(NdotH, roughness);
		float fh = std::pow(std::min(1.0f, std::max(0.0f, 1 - HdotL)), 5.0f);
		float F  = (1 - fh) * spec + fh;
		float G  = Microfacet::g_1_smith(NdotL, roughness) * Microfacet::g_1_smith(-point.NdotV, roughness);

		// 1/(4*NdotV*NdotL) already multiplied out
		return D * F * G;
	}

	inline float clearcoatTerm(const ShadingPoint& point, float gloss, float NdotL, float HdotL, float NdotH) const
	{
		// This is fixed by definition
		static float F0 = 0.04f;
		static float R  = 0.25f;

		float D  = Microfacet::ndf_ggx(NdotH, 0.1f * (1 - gloss) + 0.001f * gloss);
		float fh = std::pow(std::min(1.0f, std::max(0.0f, 1 - HdotL)), 5.0f);
		float F  = (1 - fh) * F0 + fh;
		float G  = Microfacet::g_1_smith(NdotL, R) * Microfacet::g_1_smith(-point.NdotV, R);

		// 1/(4*NdotV*NdotL) already multiplied out
		return D * F * G;
	}

	inline float sheenTerm(float sheen, float HdotL) const
	{
		float fh = std::pow(std::min(1.0f, std::max(0.0f, 1 - HdotL)), 5.0f);
		return fh * sheen;
	}

	float evalBRDF(const ShadingPoint& point, const Vector3f& L,
				   float base,
				   float subsurface,
				   float roughness,
				   float metallic,
				   float spec,
				   float specTint,
				   float sheen,
				   float sheenTint,
				   float clearcoat,
				   float clearcoatGloss) const
	{
		const Eigen::Vector3f H = Reflection::halfway(point.Ray.Direction, L);
		const float NdotL		= point.N.dot(L);
		const float NdotH		= point.N.dot(H);
		const float HdotL		= L.dot(H);

		float diffTerm = diffuseTerm(point, NdotL, HdotL, roughness);
		float ssTerm   = subsurfaceTerm(point, NdotL, HdotL, roughness);
		float specTerm = specularTerm(point, spec, NdotL, HdotL, NdotH, roughness);
		float ccTerm   = clearcoatTerm(point, clearcoatGloss, NdotL, HdotL, NdotH);
		float shTerm   = sheenTerm(sheen, HdotL);

		const float diffuse  = base * ((1 - subsurface) * diffTerm + subsurface * ssTerm);
		const float specular = specTerm * ((1 - specTint) + specTint * base)
							   + ccTerm * clearcoat;

		return diffuse * (1 - metallic)
			   + specular
			   + shTerm * (1 - metallic) * ((1 - sheenTint) + sheenTint * base);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		float base			 = mBaseColor->eval(in.Point);
		float subsurface	 = mSubsurface->eval(in.Point);
		float roughness		 = mRoughness->eval(in.Point);
		float metallic		 = mMetallic->eval(in.Point);
		float spec			 = mSpecular->eval(in.Point);
		float specTint		 = mSpecularTint->eval(in.Point);
		float sheen			 = mSheen->eval(in.Point);
		float sheenTint		 = mSheenTint->eval(in.Point);
		float clearcoat		 = mClearcoat->eval(in.Point);
		float clearcoatGloss = mClearcoatGloss->eval(in.Point);

		out.Weight = evalBRDF(in.Point, in.Outgoing,
							  base, subsurface, roughness, metallic, spec, specTint,
							  sheen, sheenTint, clearcoat, clearcoatGloss)
					 * std::abs(in.NdotL);

		float weight = 1 - roughness;
		out.PDF_S	= Projection::cos_hemi_pdf(in.NdotL) * (1 - weight) + Microfacet::pdf_ggx(in.NdotL, roughness) * weight; // Use NdotH instead of NdotL!

		if (metallic < 0.5f)
			out.Type = MST_DiffuseReflection;
		else
			out.Type = MST_SpecularReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput& in, MaterialSampleOutput& out) const
	{
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
		out.Outgoing = Tangent::align(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		out.Type	 = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, MaterialSampleOutput& out, float roughness) const
	{
		out.Outgoing = Microfacet::sample_ndf_ggx(in.RND[0], in.RND[1], roughness, out.PDF_S);
		out.Outgoing = Tangent::align(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		out.Type	 = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		float base			 = mBaseColor->eval(in.Point);
		float subsurface	 = mSubsurface->eval(in.Point);
		float roughness		 = mRoughness->eval(in.Point);
		float metallic		 = mMetallic->eval(in.Point);
		float spec			 = mSpecular->eval(in.Point);
		float specTint		 = mSpecularTint->eval(in.Point);
		float sheen			 = mSheen->eval(in.Point);
		float sheenTint		 = mSheenTint->eval(in.Point);
		float clearcoat		 = mClearcoat->eval(in.Point);
		float clearcoatGloss = mClearcoatGloss->eval(in.Point);

		float weight = 1 - roughness;
		if (in.RND[0] < weight)
			sampleSpecularPath(in, out, roughness);
		else
			sampleDiffusePath(in, out);

		out.Weight = evalBRDF(in.Point, out.Outgoing,
							  base, subsurface, roughness, metallic, spec, specTint,
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