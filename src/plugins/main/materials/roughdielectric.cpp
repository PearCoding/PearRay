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
template <bool HasTransmissionColor, bool SpectralVarying, bool HasAnisoRoughness>
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

	inline SpectralBlob fresnelTerm(const MaterialSampleContext& spt, float dot, const ShadingContext& sctx) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (spt.IsInside)
			std::swap(n1, n2);

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(dot, n1[i], n2[i]);
		return res;
	}

	inline float fresnelTermHero(const MaterialSampleContext& spt, float dot, const ShadingContext& sctx, float& eta) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (spt.IsInside)
			std::swap(n1, n2);

		eta = n1[0] / n2[0];

		return Fresnel::dielectric(dot, n1[0], n2[0]);
	}

	inline float evalGD(const ShadingVector& H, const ShadingVector& V, const ShadingVector& L, const ShadingContext& sctx, float& pdf) const
	{
		const float absNdotH = H.cosTheta();
		const float absNdotV = V(2);

		PR_ASSERT(absNdotV >= 0, "By definition N.V has to be positive");
		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		const float m1 = adaptR(mRoughnessX->eval(sctx));

		float G;
		float D;
		if constexpr (!HasAnisoRoughness) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
			G = Microfacet::g_1_smith(V, m1) * Microfacet::g_1_smith(L, m1);
		} else {
			const float m2 = adaptR(mRoughnessY->eval(sctx));
			D			   = Microfacet::ndf_ggx(H, m1, m2);
			G			   = Microfacet::g_1_smith(V, m1, m2) * Microfacet::g_1_smith(L, m1, m2);
		}

		const float factor = std::abs(H.dot(V) * H.dot(L)) / absNdotV; // NdotL multiplied out
		pdf				   = std::abs(D * absNdotH);
		return std::abs(G * D * factor);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if (in.Context.V.sameHemisphere(in.Context.L)) { // Scattering
			out.Type		  = MST_SpecularReflection;
			ShadingVector H	  = Scattering::halfway_reflection(in.Context.V, in.Context.L);
			const float HdotV = H.dot(in.Context.V);
			PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

			float pdf;
			float gd	   = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
			SpectralBlob F = fresnelTerm(in.Context, HdotV, in.ShadingContext);

			float jacobian = 1 / (4 * HdotV * HdotV);
			out.Weight	   = mSpecularity->eval(in.ShadingContext) * F * (gd * jacobian);
			out.PDF_S	   = F * pdf * jacobian;
		} else {
			SpectralBlob weight = HasTransmissionColor ? mTransmission->eval(in.ShadingContext) : mSpecularity->eval(in.ShadingContext);

			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (in.Context.IsInside)
				std::swap(n1, n2);

			SpectralBlob inv_eta = n2 / n1;

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				Vector3f H = Scattering::halfway_transmission(n1[i], in.Context.V, n2[i], in.Context.L);
				if (H(2) < 0)
					H = -H;

				float HdotV = H.dot((Vector3f)in.Context.V);
				float HdotL = H.dot((Vector3f)in.Context.L);

				float pdf;
				float gd	   = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
				float F		   = Fresnel::dielectric(HdotV, n1[i], n2[i]);
				float denom	   = HdotV + inv_eta[i] * HdotL;
				float jacobian = inv_eta[i] * inv_eta[i] / (denom * denom);

				// TODO: Deaccount for solid angle compression (see mitsuba & pbrt) when non-radiance mode is evaluated
				float factor = HdotL < 0 ? 1 / inv_eta[i] : inv_eta[i];

				out.Weight[i] = (1 - F) * weight[i] * gd * jacobian * factor * factor;
				out.PDF_S[i]  = (1 - F) * pdf * jacobian;
			}
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		// FIXME: Just a bad hack
		float u1 = int(in.RND[0] * 100) / 100.0f;
		float u2 = in.RND[0] * 100 - int(in.RND[0] * 100);

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
		out.PDF_S = pdf;

		// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
		float _pdf;
		const float gd = evalGD(H, in.Context.V, out.L, in.ShadingContext, _pdf);

		const float HdotV = H.dot((Vector3f)in.Context.V);
		float HdotL;

		// Calculate Fresnel term
		float eta;
		const float F = fresnelTermHero(in.Context, HdotV, in.ShadingContext, eta);

		if (u1 <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Scattering::reflect(in.Context.V, H);
			HdotL	 = HdotV;

			if (out.L(2) <= PR_EPSILON) { // Side check
				out.Weight = SpectralBlob::Zero();
				out.PDF_S  = 0;
				return;
			} else
				out.Weight = mSpecularity->eval(in.ShadingContext);

			out.PDF_S *= F;
		} else {
			HdotL = Scattering::refraction_angle(HdotV, eta);

			if (HdotL < 0) { // TOTAL REFLECTION
				out.Type = MST_SpecularReflection;
				out.L	 = Scattering::reflect(in.Context.V, H);
				HdotL	 = HdotV;
				if (out.L(2) <= PR_EPSILON) { // Side check
					out.Weight = SpectralBlob::Zero();
					out.PDF_S  = 0;
					return;
				} else
					out.Weight = mSpecularity->eval(in.ShadingContext);
			} else {
				out.Type = MST_SpecularTransmission;
				out.L	 = Scattering::refract(eta, HdotL, HdotV, in.Context.V, H);
				if (out.L(2) >= PR_EPSILON) { // Side check
					out.Weight = SpectralBlob::Zero();
					out.PDF_S  = 0;
					return;
				} else {
					if constexpr (HasTransmissionColor)
						out.Weight = mTransmission->eval(in.ShadingContext);
					else
						out.Weight = mSpecularity->eval(in.ShadingContext);
				}
			}

			out.PDF_S *= 1 - F;
		}

		// Apply jacobian based on actual direction
		if (out.Type == MST_SpecularReflection) {
			float jacobian = 1 / (4 * HdotV * HdotV);
			out.Weight *= gd * jacobian;
			out.PDF_S *= jacobian;
		} else {
			float inv_eta  = 1 / eta;
			float denom	   = HdotV + inv_eta * HdotL;
			float jacobian = inv_eta * inv_eta / (denom * denom);
			float factor   = HdotL < 0 ? eta : inv_eta;
			out.Weight *= gd * jacobian * factor * factor;
			out.PDF_S *= jacobian;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughDielectricMaterial>:" << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl;

		if constexpr (HasTransmissionColor)
			stream << "    Transmission:     " << mTransmission->dumpInformation() << std::endl;

		stream << "    IOR:             " << mIOR->dumpInformation() << std::endl;
		if constexpr (SpectralVarying)
			stream << "    SpectralVarying: true" << std::endl;
		stream << "    RoughnessX:      " << mRoughnessX->dumpInformation() << std::endl;
		stream << "    RoughnessY:      " << mRoughnessY->dumpInformation() << std::endl;

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
template <bool HasTransmissionColor, bool SpectralVarying, bool HasAnisoRoughness>
static std::shared_ptr<IMaterial> createMaterial1(uint32 id, const SceneLoadContext& ctx)
{
	std::shared_ptr<FloatScalarNode> rx;
	std::shared_ptr<FloatScalarNode> ry;

	if (ctx.Parameters.hasParameter("roughness_x"))
		rx = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness_x"), 0);
	else
		rx = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness"), 0);

	if constexpr (HasAnisoRoughness)
		ry = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness_y"), 0);

	return std::make_shared<RoughDielectricMaterial<HasTransmissionColor, SpectralVarying, HasAnisoRoughness>>(
		id,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("specularity"), 1),
		HasTransmissionColor ? ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("transmission"), 1) : nullptr,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("index"), 1.55f),
		rx, ry);
}

template <bool HasTransmissionColor, bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial2(uint32 id, const SceneLoadContext& ctx)
{
	const bool roughness_y = ctx.Parameters.hasParameter("roughness_y");
	if (roughness_y)
		return createMaterial1<HasTransmissionColor, SpectralVarying, true>(id, ctx);
	else
		return createMaterial1<HasTransmissionColor, SpectralVarying, false>(id, ctx);
}

template <bool HasTransmissionColor>
static std::shared_ptr<IMaterial> createMaterial3(uint32 id, const SceneLoadContext& ctx)
{
	const bool spectralVarying = ctx.Parameters.getBool("spectral_varying", true);
	if (spectralVarying)
		return createMaterial2<HasTransmissionColor, true>(id, ctx);
	else
		return createMaterial2<HasTransmissionColor, false>(id, ctx);
}

static std::shared_ptr<IMaterial> createMaterial4(uint32 id, const SceneLoadContext& ctx)
{
	const bool hasTransmission = ctx.Parameters.hasParameter("transmission");
	if (hasTransmission)
		return createMaterial3<true>(id, ctx);
	else
		return createMaterial3<false>(id, ctx);
}

class RoughDielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		// TODO: If no roughness is given -> yield to glass material
		return createMaterial4(id, ctx);
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