#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

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

	inline float evalGD(const Vector3f& H, const Vector3f& V, const Vector3f& L, const ShadingContext& sctx, float& pdf) const
	{
		const float absNdotH = std::abs(H(2));
		const float absNdotV = std::abs(V(2));
		const float absNdotL = std::abs(L(2));

		if (absNdotH <= PR_EPSILON
			|| absNdotV <= PR_EPSILON
			|| absNdotL <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		float m1 = mRoughnessX->eval(sctx);
		m1 *= m1;

		float G;
		float D;
		if constexpr (!HasAnisoRoughness) {
			D = Microfacet::ndf_ggx(absNdotH, m1);
			G = Microfacet::g_1_smith(absNdotV, m1) * Microfacet::g_1_smith(absNdotL, m1);
		} else {
			float m2 = mRoughnessY->eval(sctx);
			m2 *= m2;
			D = Microfacet::ndf_ggx(absNdotH, H(0), H(1), m1, m2);
			G = Microfacet::g_1_smith(absNdotV, V(0), V(1), m1, m2) * Microfacet::g_1_smith(absNdotL, L(0), L(1), m1, m2);
		}

		pdf = D * absNdotH;
		return G * D;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if (in.Context.V.sameHemisphere(in.Context.L)) { // Reflection
			out.Type	= MST_SpecularReflection;
			float HdotV = in.Context.HdotV();
			PR_ASSERT(HdotV >= 0.0f, "HdotV must be positive");

			float pdf;
			float gd	   = evalGD(in.Context.H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
			SpectralBlob F = fresnelTerm(in.Context, HdotV, in.ShadingContext);
			out.Weight	   = mSpecularity->eval(in.ShadingContext) * F * (gd / (4 * in.Context.NdotV()));
			out.PDF_S	   = F * pdf / (4 * in.Context.HdotL());
		} else {
			SpectralBlob weight = HasTransmissionColor ? mTransmission->eval(in.ShadingContext) : mSpecularity->eval(in.ShadingContext);

			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = mIOR->eval(in.ShadingContext);

			if (in.Context.IsInside)
				std::swap(n1, n2);

			SpectralBlob eta = n1 / n2;

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				Vector3f H = ((Vector3f)in.Context.V + (Vector3f)in.Context.L * eta[i]).normalized();
				if (in.Context.IsInside)
					H = -H;

				if (H(2) < 0)
					H = -H;

				float HdotV = std::abs(H.dot((Vector3f)in.Context.V));
				float HdotL = H.dot((Vector3f)in.Context.L);
				float denom = HdotV + eta[i] * HdotL;

				float pdf;
				float gd = evalGD(H, in.Context.V, in.Context.L, in.ShadingContext, pdf);
				float F	 = Fresnel::dielectric(HdotV, n1[i], n2[i]);

				// Deaccount for solid angle compression (see mitsuba & pbrt) when non-radiance mode is evaluated
				out.Weight[i] = (1 - F) * weight[i] * gd * HdotV * HdotL / (std::abs(in.Context.NdotV()) * denom * denom);
				out.PDF_S[i]  = (1 - F) * pdf * eta[i] * eta[i] * HdotL / (denom * denom);
			}
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		// FIXME: Just a bad hack
		float u1 = int(in.RND[0] * 100) / 100.0f;
		float u2 = in.RND[0] * 100 - int(in.RND[0] * 100);

		float m1 = mRoughnessX->eval(in.ShadingContext);
		m1 *= m1;
		float pdf = 1;
		Vector3f H;
		if constexpr (HasAnisoRoughness) {
			float m2 = mRoughnessY->eval(in.ShadingContext);
			m2 *= m2;
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

		const float HdotV = std::abs(H.dot((Vector3f)in.Context.V));
		float HdotL;

		float eta;
		float F = fresnelTermHero(in.Context, HdotV, in.ShadingContext, eta);

		if (u1 <= F) {
			out.PDF_S *= F;
			out.Type = MST_SpecularReflection;
			out.L	 = Reflection::reflect(in.Context.V, H);
			HdotL	 = HdotV;
			if (out.L(2) <= PR_EPSILON) { // Side check
				out.Weight = SpectralBlob::Zero();
				out.PDF_S  = 0;
				return;
			} else
				out.Weight = mSpecularity->eval(in.ShadingContext);
		} else {
			out.PDF_S *= 1 - F;
			HdotL = Reflection::refraction_angle(HdotV, eta);

			if (HdotL < 0) { // TOTAL REFLECTION
				out.Type = MST_SpecularReflection;
				out.L	 = Reflection::reflect(in.Context.V, H);
				HdotL	 = HdotV;
				if (out.L(2) <= PR_EPSILON) { // Side check
					out.Weight = SpectralBlob::Zero();
					out.PDF_S  = 0;
					return;
				} else
					out.Weight = mSpecularity->eval(in.ShadingContext);
			} else {
				out.Type = MST_SpecularTransmission;
				out.L	 = Reflection::refract(eta, HdotL, HdotV, in.Context.V, H);
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
		}

		float _pdf;
		const float gd = std::abs(evalGD(H, in.Context.V, out.L, in.ShadingContext, _pdf));
		out.Weight *= gd * HdotV / (pdf * std::abs(in.Context.NdotV()));

		if (out.Type == MST_SpecularReflection) {
			out.PDF_S /= 4 * HdotL;
		} else {
			float denom = HdotV + eta * HdotL;
			out.Weight *= eta * eta;
			out.PDF_S *= eta * eta * HdotL / (denom * denom);
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