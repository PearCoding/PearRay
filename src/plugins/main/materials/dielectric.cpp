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

template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying, bool HasRoughness, bool HasAnisoRoughness>
class DielectricMaterial : public IMaterial {
public:
	DielectricMaterial(uint32 id,
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

	virtual ~DielectricMaterial() = default;

	int flags() const override { return (HasRoughness ? 0 : MF_DeltaDistribution) | (SpectralVarying ? MF_SpectralVarying : 0); }

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

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		if constexpr (HasTransmissionColor) {
			SpectralBlob spec  = mSpecularity->eval(in.ShadingContext);
			SpectralBlob trans = mTransmission->eval(in.ShadingContext);
			SpectralBlob F	   = fresnelTerm(in.Context, HasRoughness ? in.Context.HdotV() : in.Context.NdotV(), in.ShadingContext);
			if constexpr (IsThin) {
				// Account for scattering between interfaces
				F += (F < 1.0f).select((1 - F) * F / (F + 1), 0);
			}

			out.Weight = spec * F + trans * (1 - F);
		} else {
			out.Weight = mSpecularity->eval(in.ShadingContext);
		}

		if constexpr (HasRoughness && !HasAnisoRoughness)
			out.PDF_S = Microfacet::pdf_ggx(in.Context.NdotH(), mRoughnessX->eval(in.ShadingContext));
		else if constexpr (HasRoughness && HasAnisoRoughness)
			out.PDF_S = Microfacet::pdf_ggx(in.Context.NdotH(), mRoughnessX->eval(in.ShadingContext), mRoughnessY->eval(in.ShadingContext));
		else
			out.PDF_S = 1;

		if (in.Context.NdotL() < 0)
			out.Type = MST_SpecularTransmission;
		else
			out.Type = MST_SpecularReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// FIXME: Just a bad hack
		float u1 = int(in.RND[0] * 100) / 100.0f;
		float u2 = in.RND[0] * 100 - int(in.RND[0] * 100);

		float pdf = 1;
		[[maybe_unused]] Vector3f H;
		if constexpr (HasRoughness && HasAnisoRoughness)
			H = Microfacet::sample_ndf_ggx(u1, in.RND[1], mRoughnessX->eval(in.ShadingContext), mRoughnessY->eval(in.ShadingContext), pdf);
		else if constexpr (HasRoughness && !HasAnisoRoughness)
			H = Microfacet::sample_ndf_ggx(u1, in.RND[1], mRoughnessX->eval(in.ShadingContext), pdf);
		out.PDF_S = pdf;

		float eta;
		float F = fresnelTermHero(in.Context, HasRoughness ? in.Context.V.dot(H) : in.Context.NdotV(), in.ShadingContext, eta);
		if constexpr (IsThin) {
			// Account for scattering between interfaces
			if (F < 1.0f)
				F += (1 - F) * F / (F + 1);
		}

		if (u2 <= F) {
			out.Type = MST_SpecularReflection;
			if constexpr (HasRoughness)
				out.L = Reflection::reflect(in.Context.V, H);
			else
				out.L = Reflection::reflect(in.Context.V);
		} else {
			if constexpr (IsThin) {
				out.Type = MST_SpecularTransmission;
				out.L	 = -in.Context.V;
			} else {
				if constexpr (HasRoughness) {
					const float HdotT = Reflection::refraction_angle(in.Context.V.dot(H), eta);

					if (HdotT < 0) { // TOTAL REFLECTION
						out.Type = MST_SpecularReflection;
						out.L	 = Reflection::reflect(in.Context.V, H);
					} else {
						out.Type = MST_SpecularTransmission;
						out.L	 = Reflection::refract(eta, HdotT, in.Context.V);
					}
				} else {
					const float NdotT = Reflection::refraction_angle(in.Context.NdotV(), eta);

					if (NdotT < 0) { // TOTAL REFLECTION
						out.Type = MST_SpecularReflection;
						out.L	 = Reflection::reflect(in.Context.V);
					} else {
						out.Type = MST_SpecularTransmission;
						out.L	 = Reflection::refract(eta, NdotT, in.Context.V);
					}
				}
			}
		}
		// The weight is independent of the fresnel term
		if constexpr (HasTransmissionColor) {
			if (out.Type == MST_SpecularReflection)
				out.Weight = mSpecularity->eval(in.ShadingContext);
			else
				out.Weight = mTransmission->eval(in.ShadingContext);
		} else {
			out.Weight = mSpecularity->eval(in.ShadingContext);
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DielectricMaterial>:" << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl;

		if constexpr (HasTransmissionColor)
			stream << "    Transmission:     " << mTransmission->dumpInformation() << std::endl;

		stream << "    IOR:             " << mIOR->dumpInformation() << std::endl;
		if constexpr (IsThin)
			stream << "    IsThin:          true" << std::endl;
		if constexpr (SpectralVarying)
			stream << "    SpectralVarying: true" << std::endl;
		if constexpr (HasRoughness) {
			stream << "    RoughnessX:      " << mRoughnessX->dumpInformation() << std::endl;
			stream << "    RoughnessY:      " << mRoughnessY->dumpInformation() << std::endl;
		}
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
template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying, bool HasRoughness, bool HasAnisoRoughness>
static std::shared_ptr<IMaterial> createMaterial1(uint32 id, const SceneLoadContext& ctx)
{
	std::shared_ptr<FloatScalarNode> rx;
	std::shared_ptr<FloatScalarNode> ry;

	if constexpr (HasRoughness) {
		if (ctx.Parameters.hasParameter("roughness_x"))
			rx = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness_x"), 0);
		else
			rx = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness"), 0);

		if constexpr (HasAnisoRoughness)
			ry = ctx.Env->lookupScalarNode(ctx.Parameters.getParameter("roughness_y"), 0);
	}

	return std::make_shared<DielectricMaterial<HasTransmissionColor, IsThin, SpectralVarying, HasRoughness, HasAnisoRoughness>>(
		id,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("specularity"), 1),
		HasTransmissionColor ? ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("transmission"), 1) : nullptr,
		ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter("index"), 1.55f),
		rx, ry);
}

template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying, bool HasRoughness>
static std::shared_ptr<IMaterial> createMaterial2(uint32 id, const SceneLoadContext& ctx)
{
	if constexpr (HasRoughness) {
		const bool roughness_y = ctx.Parameters.hasParameter("roughness_y");
		if (roughness_y)
			return createMaterial1<HasTransmissionColor, IsThin, SpectralVarying, HasRoughness, true>(id, ctx);
		else
			return createMaterial1<HasTransmissionColor, IsThin, SpectralVarying, HasRoughness, false>(id, ctx);
	} else {
		return createMaterial1<HasTransmissionColor, IsThin, SpectralVarying, false, false>(id, ctx);
	}
}

template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial3(uint32 id, const SceneLoadContext& ctx)
{
	const bool roughness   = ctx.Parameters.hasParameter("roughness");
	const bool roughness_x = ctx.Parameters.hasParameter("roughness_x");
	if (roughness || roughness_x)
		return createMaterial2<HasTransmissionColor, IsThin, SpectralVarying, true>(id, ctx);
	else
		return createMaterial2<HasTransmissionColor, IsThin, SpectralVarying, false>(id, ctx);
}

template <bool HasTransmissionColor, bool IsThin>
static std::shared_ptr<IMaterial> createMaterial4(uint32 id, const SceneLoadContext& ctx)
{
	const bool spectralVarying = ctx.Parameters.getBool("spectral_varying", true);
	if (spectralVarying)
		return createMaterial3<HasTransmissionColor, IsThin, true>(id, ctx);
	else
		return createMaterial3<HasTransmissionColor, IsThin, false>(id, ctx);
}

template <bool HasTransmissionColor>
static std::shared_ptr<IMaterial> createMaterial5(uint32 id, const SceneLoadContext& ctx)
{
	const bool isThin = ctx.Parameters.getBool("thin", false);
	if (isThin)
		return createMaterial4<HasTransmissionColor, true>(id, ctx);
	else
		return createMaterial4<HasTransmissionColor, false>(id, ctx);
}

static std::shared_ptr<IMaterial> createMaterial6(uint32 id, const SceneLoadContext& ctx)
{
	const bool hasTransmission = ctx.Parameters.hasParameter("transmission");
	if (hasTransmission)
		return createMaterial5<true>(id, ctx);
	else
		return createMaterial5<false>(id, ctx);
}

class DielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		return createMaterial6(id, ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "glass", "dielectric" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DielectricMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)