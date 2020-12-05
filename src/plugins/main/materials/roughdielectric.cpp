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
 *   K = V.H*n2^2 /(n1*(V.H)+n2*(L.H))^2 // Jacobian
 * 
 * To be precise the jacobian is already multiplied by 1/(V.H)
 * and the whole BSDF is multiplied by |L.N| therefore the L.N in the denominator disappears
 * 
 * Notice: eta is always n1/n2 with n1 being the IOR of V and n2 the IOR of L. This is inconsistent within Mitsuba and PBRT
 * 
 * Based on:
 * Walter, Bruce & Marschner, Stephen & Li, Hongsong & Torrance, Kenneth. (2007). Microfacet Models for Refraction through Rough Surfaces.. 
 * Eurographics Symposium on Rendering. 195-206. 10.2312/EGWR/EGSR07/195-206. 
 */

// TODO: Roughness + Thin
template <bool SpectralVarying, bool UseVNDF, bool IsAnisotropic>
class RoughDielectricMaterial : public IMaterial {
public:
	RoughDielectricMaterial(const std::shared_ptr<FloatSpectralNode>& spec,
							const std::shared_ptr<FloatSpectralNode>& trans,
							const std::shared_ptr<FloatSpectralNode>& ior,
							const std::shared_ptr<FloatScalarNode>& roughnessX,
							const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial()
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
		, mRoughness(roughnessX, roughnessY)
	{
	}

	virtual ~RoughDielectricMaterial() = default;

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

	inline static float reflectiveJacobian(float HdotV)
	{
		return 1 / (4 * HdotV * HdotV);
	}

	inline static float refractiveJacobian(float HdotV, float HdotL, float inv_eta)
	{
		const float denom = inv_eta * HdotL + HdotV; // Unsigned length of (unnormalized) refractive H
		return HdotV / (denom * denom);
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
			const float gd		 = mRoughness.eval(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
			const SpectralBlob F = fresnelTerm(HdotV, in.ShadingContext);

			const float jacobian = reflectiveJacobian(HdotV);
			out.Weight			 = mSpecularity->eval(in.ShadingContext) * F * (gd * jacobian);
			out.PDF_S			 = F * pdf * jacobian;
		} else {
			const bool flip		  = in.Context.V.cosTheta() < 0;
			const ShadingVector V = flip ? -in.Context.V : in.Context.V;
			const ShadingVector L = flip ? -in.Context.L : in.Context.L;

			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (flip)
				std::swap(n1, n2);

			// Mitsuba style eta definition
			const SpectralBlob inv_eta = n2 / n1;

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const Vector3f H = Scattering::halfway_transmission(inv_eta[i], V, L);

				// The refractive H vector does not have the good property of HdotV == HdotL as in the reflective case:
				float HdotV = H.dot((Vector3f)V);
				float HdotL = H.dot((Vector3f)L);

				if (HdotV * HdotL >= 0) { // Same side
					out.Weight[i] = 0;
					out.PDF_S[i]  = 0;
					continue;
				}

				float _pdf;
				const float gd		 = mRoughness.eval(H, in.Context.V, in.Context.L, in.ShadingContext, _pdf);
				const float jacobian = refractiveJacobian(HdotV, -HdotL, inv_eta[i]);
				const float F		 = Fresnel::dielectric(HdotV, -HdotL, n1[i], n2[i]); // Zero in case of total reflection

				out.Weight[i] = (1 - F) * gd * jacobian;
				out.PDF_S[i]  = (1 - F) * _pdf * jacobian;
			}

			out.Weight *= mTransmission->eval(in.ShadingContext);

			// Only rays from lights are weighted by this factor
			// as radiance flows in the opposite direction
			// Note that some implementations use 1/eta as eta
			if ((in.Context.RayFlags & RF_Light))
				out.Weight *= inv_eta * inv_eta;
		}
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

			const float pdf		 = mRoughness.pdf(H, in.Context.V, in.ShadingContext);
			const SpectralBlob F = fresnelTerm(HdotV, in.ShadingContext);

			const float jacobian = reflectiveJacobian(HdotV);
			out.PDF_S			 = F * pdf * jacobian;
		} else {
			const bool flip		  = in.Context.V.cosTheta() < 0;
			const ShadingVector V = flip ? -in.Context.V : in.Context.V;
			const ShadingVector L = flip ? -in.Context.L : in.Context.L;

			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (flip)
				std::swap(n1, n2);

			// Mitsuba style eta definition
			const SpectralBlob inv_eta = n2 / n1;

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const Vector3f H = Scattering::halfway_transmission(inv_eta[i], V, L);

				float HdotV = H.dot((Vector3f)V);
				float HdotL = H.dot((Vector3f)L);

				if (HdotV * HdotL >= 0) { // Same side
					out.PDF_S[i] = 0;
					continue;
				}

				const float pdf		 = mRoughness.pdf(H, in.Context.V, in.ShadingContext);
				const float jacobian = refractiveJacobian(HdotV, -HdotL, inv_eta[i]);
				const float F		 = Fresnel::dielectric(HdotV, -HdotL, n1[i], n2[i]); // Zero in case of total reflection

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

		const bool flip		  = in.Context.V.cosTheta() < 0;
		const ShadingVector V = flip ? -in.Context.V : in.Context.V;

		// Sample microfacet normal
		float pdf;
		bool delta;
		const Vector3f H = mRoughness.sample(Vector2f(u2, in.RND[1]), V, in.ShadingContext, pdf, delta);
		out.PDF_S		 = pdf;

		// Set flags
		if constexpr (SpectralVarying)
			out.Flags = (delta ? MSF_DeltaDistribution : 0) | MSF_SpectralVarying;
		else
			out.Flags = (delta ? MSF_DeltaDistribution : 0);

		// Check correct side
		const float HdotV = H.dot((Vector3f)V);
		if (HdotV <= PR_EPSILON) {
			out = MaterialSampleOutput::Reject(MST_SpecularReflection, out.Flags);
			return;
		}

		// Calculate Fresnel term
		float eta;
		float HdotT;
		const float F			   = fresnelTermHero(HdotV, in.ShadingContext, eta, HdotT);
		const bool totalReflection = HdotT < 0;

		// Breach out into two cases
		if (totalReflection || u1 <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Scattering::reflect(in.Context.V, H); // No need to use the (possibly) flipped V

			if (!in.Context.V.sameHemisphere(out.L)) { // Side check
				out = MaterialSampleOutput::Reject(MST_SpecularReflection, out.Flags);
				return;
			}

			out.Weight = mSpecularity->eval(in.ShadingContext);

			if (delta) {
				// Evaluate D*G*(V.H*L.H)/(V.N) but ignore pdf
				float _pdf;
				const float gd		 = mRoughness.eval(H, in.Context.V, out.L, in.ShadingContext, _pdf);
				const float jacobian = reflectiveJacobian(HdotV);
				out.Weight *= F * gd * jacobian;
				out.PDF_S *= F * jacobian;
			}
		} else {
			out.Type = MST_SpecularTransmission;
			out.L	 = Scattering::refract(eta, HdotT, HdotV, V, H);

			// Side check: As we have a refraction case, both vectors should not be on the same side!
			if (in.Context.V.sameHemisphere(out.L)) {
				out = MaterialSampleOutput::Reject(MST_SpecularTransmission, out.Flags);
				return;
			}

			out.Weight = mTransmission->eval(in.ShadingContext);

			if (!delta) {
				// The refractive H vector does not have the good property of HdotV == HdotL as in the reflective case:
				const float HdotL = H.dot((Vector3f)out.L);

				PR_ASSERT(HdotV * HdotL < 0, "V & L have to be on different sides by construction");

				float _pdf; // Ignore
				const float gd		 = mRoughness.eval(H, V, out.L, in.ShadingContext, _pdf);
				const float jacobian = refractiveJacobian(HdotV, -HdotL, 1 / eta);

				out.Weight *= (1 - F) * gd * jacobian;
				out.PDF_S *= (1 - F) * jacobian;

				PR_ASSERT(out.Weight[0]/out.PDF_S[0] <= 1, "Expected correct weights");
			}

			if (flip)
				out.L = -out.L;

			// Only rays from lights are weighted by this factor
			// as radiance flows in the opposite direction
			// Note that some implementations use 1/eta as eta
			if ((in.Context.RayFlags & RF_Light))
				out.Weight /= eta * eta;
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
			   << "    RoughnessX:   " << mRoughness.roughnessX()->dumpInformation() << std::endl
			   << "    RoughnessY:   " << mRoughness.roughnessY()->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatSpectralNode> mTransmission;
	const std::shared_ptr<FloatSpectralNode> mIOR;
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

	auto spec  = ctx.lookupSpectralNode("specularity", 1);
	auto trans = spec;
	if (ctx.parameters().hasParameter("transmission"))
		trans = ctx.lookupSpectralNode("transmission", 1);

	const auto index = ctx.lookupSpectralNode({ "eta", "index", "ior" }, 1.55f);

	if (rx == ry)
		return std::make_shared<RoughDielectricMaterial<SpectralVarying, UseVNDF, false>>(spec, trans, index, rx, ry);
	else
		return std::make_shared<RoughDielectricMaterial<SpectralVarying, UseVNDF, true>>(spec, trans, index, rx, ry);
}

template <bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial2(const SceneLoadContext& ctx)
{
	// TODO: Fix VNDF and make it default again
	const bool use_vndf = ctx.parameters().getBool("vndf", false);

	if (use_vndf)
		return createMaterial1<SpectralVarying, true>(ctx);
	else
		return createMaterial1<SpectralVarying, true>(ctx);
}

static std::shared_ptr<IMaterial> createMaterial3(const SceneLoadContext& ctx)
{
	const bool spectralVarying = ctx.parameters().getBool("spectral_varying", true);
	if (spectralVarying)
		return createMaterial2<true>(ctx);
	else
		return createMaterial2<false>(ctx);
}

class RoughDielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		return createMaterial3(ctx);
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