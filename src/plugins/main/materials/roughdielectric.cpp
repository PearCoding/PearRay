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

#include <sstream>

namespace PR {

/* Refraction based microfacets
 * Reflection:
 *   F*D*G*(V.H*L.H)/(V.N*L.N)*K
 *   K = 1/(4*(V.H)^2) // Jacobian
 * 
 * Notice: Due to law of reflection it is V.H = L.H => (V.H*L.H)/(V.H)^2 = 1
 * but for compliance with the refraction brach this is not optimized
 *
 * Refraction:
 *   (1-F)*D*G*(V.H*L.H)/(V.N*L.N)*K
 *   K = n2^2 /(n1*(V.H)+n2*(L.H))^2 // Jacobian
 * 
 * To be precise the jacobian is already multiplied by 1/(V.H)
 * and the whole BSDF is multiplied by |L.N| therefore the L.N in the denominator disappears
 * 
 * Notice: eta is always n1/n2 with n1 being the IOR of V and n2 the IOR of L. This is inconsistent within Mitsuba and PBRT
 */

#define SQUARE_ROUGHNESS
inline static float adaptR(float r)
{
#ifdef SQUARE_ROUGHNESS
	return r * r;
#else
	return r;
#endif
}

// TODO: Roughness + Thin
template <bool SpectralVarying, bool HasAnisoRoughness>
class RoughDielectricMaterial : public IMaterial {
public:
	RoughDielectricMaterial(uint32 id,
							const std::shared_ptr<FloatSpectralNode>& spec,
							const std::shared_ptr<FloatSpectralNode>& trans,
							const std::shared_ptr<FloatSpectralNode>& ior,
							const std::shared_ptr<FloatScalarNode>& roughnessX,
							const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial(id)
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~RoughDielectricMaterial() = default;

	int flags() const override { return (SpectralVarying ? MF_SpectralVarying : 0); }

	inline SpectralBlob fresnelTerm(float dot, const ShadingContext& sctx) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (dot < 0)
			std::swap(n1, n2);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(std::abs(dot), n1[i], n2[i]);
		return res;
	}

	inline float fresnelTermHero(float dot, const ShadingContext& sctx, float& eta, float& NdotT) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (dot < 0)
			std::swap(n1, n2);

		eta	  = n1[0] / n2[0]; // See top note
		NdotT = Scattering::refraction_angle(std::abs(dot), eta);

		return Fresnel::dielectric(dot, NdotT, n1[0], n2[0]);
	}

	inline float evalGD(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L, const ShadingContext& sctx, float& pdf) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		const float m1 = adaptR(mRoughnessX->eval(sctx));
		if (m1 < PR_EPSILON) {
			pdf = 1;
			return 1;
		}

		float G;
		float D;
		if constexpr (!HasAnisoRoughness) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
			G = Microfacet::g_1_smith(V, m1) * Microfacet::g_1_smith(L, m1);
		} else {
			const float m2 = adaptR(mRoughnessY->eval(sctx));
			if (m2 < PR_EPSILON) {
				pdf = 1;
				return 1;
			}
			D = Microfacet::ndf_ggx(H, m1, m2);
			G = Microfacet::g_1_smith(V, m1, m2) * Microfacet::g_1_smith(L, m1, m2);
		}

		const float factor = std::abs(H.dot(V) * H.dot(L)) / absNdotV; // NdotL multiplied out
		pdf				   = std::abs(D * absNdotH);
		return G * D * factor;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if (in.Context.V.sameHemisphere(in.Context.L)) { // Scattering
			out.Type			  = MST_SpecularReflection;
			const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
			const float HdotV	  = H.dot(in.Context.V);
			PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

			float pdf;
			const float gd		 = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
			const SpectralBlob F = fresnelTerm(HdotV, in.ShadingContext);

			const float jacobian = 1 / (4 * HdotV * HdotV);
			out.Weight			 = mSpecularity->eval(in.ShadingContext) * F * (gd * jacobian);
			out.PDF_S			 = F * pdf * jacobian;
		} else {
			const SpectralBlob weight = mTransmission->eval(in.ShadingContext);

			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (in.Context.V.cosTheta() < 0)
				std::swap(n1, n2);

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				Vector3f H = Scattering::halfway_transmission(n1[i], in.Context.V, n2[i], in.Context.L);
				if (H(2) < 0)
					H = -H;

				const float HdotV = H.dot((Vector3f)in.Context.V);
				const float HdotL = H.dot((Vector3f)in.Context.L);

				if (HdotV * HdotL >= 0) { // Same side
					out.Weight[i] = 0;
					out.PDF_S[i]  = 0;
				}

				float pdf;
				const float gd		 = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
				const float F		 = Fresnel::dielectric(HdotV, -HdotL, n1[i], n2[i]); // Zero in case of total reflection
				const float denom	 = n2[i] * HdotL + n1[i] * HdotV;					 // Unsigned length of (unnormalized) H
				const float jacobian = n2[i] * n2[i] / (denom * denom);

				// TODO: Deaccount for solid angle compression (see mitsuba & pbrt) when non-radiance mode is evaluated
				const float factor = n2[i] / n1[i];

				out.Weight[i] = (1 - F) * weight[i] * gd * jacobian * factor * factor;
				out.PDF_S[i]  = (1 - F) * pdf * jacobian;
			}
		}
	}

	inline float pdfGD(const ShadingVector& H, const ShadingVector& V, const ShadingContext& sctx) const
	{
		const float absNdotH = H.absCosTheta();
		const float absNdotV = V.absCosTheta();

		//PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON)
			return 0.0f;

		const float m1 = adaptR(mRoughnessX->eval(sctx));
		if (m1 < PR_EPSILON)
			return 1;

		float D;
		if constexpr (!HasAnisoRoughness) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
		} else {
			const float m2 = adaptR(mRoughnessY->eval(sctx));
			if (m2 < PR_EPSILON)
				return 1;
			D = Microfacet::ndf_ggx(H, m1, m2);
		}

		return std::abs(D * absNdotH);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		// TODO: Better?
		PR_PROFILE_THIS;

		if (in.Context.V.sameHemisphere(in.Context.L)) { // Scattering
			const ShadingVector H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
			const float HdotV	  = H.dot(in.Context.V);
			PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

			const float pdf		 = pdfGD(H, in.Context.V, in.ShadingContext);
			const SpectralBlob F = fresnelTerm(HdotV, in.ShadingContext);

			const float jacobian = 1 / (4 * HdotV * HdotV);
			out.PDF_S			 = F * pdf * jacobian;
		} else {
			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (in.Context.V.cosTheta() < 0)
				std::swap(n1, n2);

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				Vector3f H = Scattering::halfway_transmission(n1[i], in.Context.V, n2[i], in.Context.L);
				if (H(2) < 0)
					H = -H;

				const float HdotV = H.dot((Vector3f)in.Context.V);
				const float HdotL = H.dot((Vector3f)in.Context.L);

				if (HdotV * HdotL >= 0) { // Same side
					out.PDF_S[i] = 0;
					continue;
				}

				const float pdf		 = pdfGD(H, in.Context.V, in.ShadingContext);
				const float F		 = Fresnel::dielectric(HdotV, -HdotL, n1[i], n2[i]); // Zero in case of total reflection
				const float denom	 = n2[i] * HdotL + n1[i] * HdotV;					 // Unsigned length of (unnormalized) H
				const float jacobian = n2[i] * n2[i] / (denom * denom);

				out.PDF_S[i] = (1 - F) * pdf * jacobian;
			}
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		// FIXME: Just a bad hack
		constexpr int SPLIT_F = 1000;
		const float u1		  = int(in.RND[0] * SPLIT_F) / float(SPLIT_F);
		const float u2		  = in.RND[0] * SPLIT_F - int(in.RND[0] * SPLIT_F);

		// Sample microfacet normal
		const float m1 = adaptR(mRoughnessX->eval(in.ShadingContext));
		float pdf	   = 1;
		Vector3f H;
		if constexpr (HasAnisoRoughness) {
			const float m2 = adaptR(mRoughnessY->eval(in.ShadingContext));
			if (m1 > PR_EPSILON && m2 > PR_EPSILON)
				H = Microfacet::sample_ndf_ggx(u2, in.RND[1], m1, m2, pdf);
			else
				H = Vector3f(0, 0, 1);
		} else {
			if (m1 > PR_EPSILON)
				H = Microfacet::sample_ndf_ggx(u2, in.RND[1], m1, pdf);
			else
				H = Vector3f(0, 0, 1);
		}

		const float HdotV = std::abs(H.dot((Vector3f)in.Context.V));
		if (HdotV <= PR_EPSILON) { // Giveup if V in respect to H is too close to the negative hemisphere
			out.Weight = SpectralBlob::Zero();
			out.PDF_S  = 0;
			return;
		}

		// Calculate Fresnel term
		float eta;
		float HdotT;
		const float F			   = fresnelTermHero(HdotV, in.ShadingContext, eta, HdotT);
		const bool totalReflection = HdotT < 0;

		// Breach out for two cases
		if (totalReflection || u1 <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Scattering::reflect(in.Context.V, H);

			if (out.L(2) <= PR_EPSILON) { // Side check
				out.Weight = SpectralBlob::Zero();
				out.PDF_S  = 0;
				return;
			}

			const float prob	 = totalReflection ? 1 : F;
			const float jacobian = 1 / (4 * HdotV * HdotV);

			// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
			float _pdf;
			const float gd = evalGD(H, in.Context.V, out.L, in.ShadingContext, _pdf);

			out.Weight = mSpecularity->eval(in.ShadingContext) * prob * gd * jacobian;
			out.PDF_S  = prob * pdf * jacobian;
		} else {
			out.Type = MST_SpecularTransmission;
			out.L	 = Scattering::refract(eta, HdotT, HdotV, in.Context.V, H);
			if (out.L(2) >= PR_EPSILON) { // Side check
				out.Weight = SpectralBlob::Zero();
				out.PDF_S  = 0;
				return;
			}

			const float HdotL = -HdotT; //out.L.dot(H);
			PR_ASSERT(HdotL <= 0, "HdotL must be negative");
			const float denom	 = HdotL + eta * HdotV; // Unsigned length of (unnormalized) H divided by n2
			const float jacobian = 1 / (denom * denom);
			const float factor	 = 1 / eta;

			// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
			float _pdf;
			const float gd = evalGD(H, in.Context.V, out.L, in.ShadingContext, _pdf);

			out.Weight = mTransmission->eval(in.ShadingContext) * (1 - F) * gd * jacobian * factor * factor;
			out.PDF_S  = (1 - F) * pdf * jacobian;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughDielectricMaterial>:" << std::endl
			   << "    Specularity:  " << mSpecularity->dumpInformation() << std::endl
			   << "    Transmission: " << mTransmission->dumpInformation() << std::endl
			   << "    IOR:          " << mIOR->dumpInformation() << std::endl
			   << "    RoughnessX:   " << mRoughnessX->dumpInformation() << std::endl
			   << "    RoughnessY:   " << mRoughnessY->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatSpectralNode> mTransmission;
	std::shared_ptr<FloatSpectralNode> mIOR;
	std::shared_ptr<FloatScalarNode> mRoughnessX;
	std::shared_ptr<FloatScalarNode> mRoughnessY;
};

// System of function which probably could be simplified with template meta programming
template <bool SpectralVarying, bool HasAnisoRoughness>
static std::shared_ptr<IMaterial> createMaterial1(uint32 id, const SceneLoadContext& ctx)
{
	std::shared_ptr<FloatScalarNode> rx;
	std::shared_ptr<FloatScalarNode> ry;

	if (ctx.parameters().hasParameter("roughness_x"))
		rx = ctx.lookupScalarNode("roughness_x", 0);
	else
		rx = ctx.lookupScalarNode("roughness", 0);

	if constexpr (HasAnisoRoughness)
		ry = ctx.lookupScalarNode("roughness_y", 0);
	else
		ry = rx;

	auto spec  = ctx.lookupSpectralNode("specularity", 1);
	auto trans = spec;
	if (ctx.parameters().hasParameter("transmission"))
		trans = ctx.lookupSpectralNode("transmission", 1);

	return std::make_shared<RoughDielectricMaterial<SpectralVarying, HasAnisoRoughness>>(
		id,
		spec, trans,
		ctx.lookupSpectralNode("index", 1.55f),
		rx, ry);
}

template <bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial2(uint32 id, const SceneLoadContext& ctx)
{
	const bool roughness_y = ctx.parameters().hasParameter("roughness_y");
	if (roughness_y)
		return createMaterial1<SpectralVarying, true>(id, ctx);
	else
		return createMaterial1<SpectralVarying, false>(id, ctx);
}

static std::shared_ptr<IMaterial> createMaterial3(uint32 id, const SceneLoadContext& ctx)
{
	const bool spectralVarying = ctx.parameters().getBool("spectral_varying", true);
	if (spectralVarying)
		return createMaterial2<true>(id, ctx);
	else
		return createMaterial2<false>(id, ctx);
}

class RoughDielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		// TODO: If no roughness is given -> yield to glass material
		return createMaterial3(id, ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "roughglass", "roughdielectric", "rough_glass", "rough_dielectric" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RoughDielectricMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)