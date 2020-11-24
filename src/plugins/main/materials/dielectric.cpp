#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Scattering.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying>
class DielectricMaterial : public IMaterial {
public:
	DielectricMaterial(uint32 id,
					   const std::shared_ptr<FloatSpectralNode>& spec,
					   const std::shared_ptr<FloatSpectralNode>& trans,
					   const std::shared_ptr<FloatSpectralNode>& ior)
		: IMaterial(id)
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
	{
	}

	virtual ~DielectricMaterial() = default;

	int flags() const override { return MF_DeltaDistribution | (SpectralVarying ? MF_SpectralVarying : 0); }

	inline float fresnelTermHero(float dot, const ShadingContext& sctx, float& eta) const
	{
		SpectralBlob n1 = SpectralBlob::Ones();
		SpectralBlob n2 = mIOR->eval(sctx);

		if (dot < 0)
			std::swap(n1, n2);

		eta = n1[0] / n2[0];

		return Fresnel::dielectric(std::abs(dot), n1[0], n2[0]);
	}

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Delta distributions do not allow evaluation
		out.PDF_S  = 0.0f;
		out.Type   = MST_SpecularTransmission;
		out.Weight = SpectralBlob::Zero();
	}

	void pdf(const MaterialEvalInput&, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Delta distributions do not allow evaluation
		out.PDF_S = 0.0f;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.PDF_S = 1;

		float eta;
		float F = fresnelTermHero(in.Context.NdotV(), in.ShadingContext, eta);

		if constexpr (IsThin) {
			// Account for scattering between interfaces
			if (F < 1.0f)
				F += (1 - F) * F / (F + 1);
		}

		if (in.RND[0] <= F) {
			out.Type = MST_SpecularReflection;
			out.L	 = Scattering::reflect(in.Context.V);
		} else {
			if constexpr (IsThin) {
				out.Type = MST_SpecularTransmission;
				out.L	 = -in.Context.V;
			} else {
				const float NdotT = Scattering::refraction_angle(std::abs(in.Context.NdotV()), eta);

				if (NdotT < 0) { // TOTAL REFLECTION
					out.Type = MST_SpecularReflection;
					out.L	 = Scattering::reflect(in.Context.V);
				} else {
					out.Type = MST_SpecularTransmission;
					out.L	 = Scattering::refract(eta, NdotT, Scattering::faceforward(in.Context.V));
					if (in.Context.NdotV() < 0)
						out.L = -out.L;
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

		// Only rays from the camera are weighted by this factor
		// as radiance flows in the opposite direction
		// Note that some implementations use 1/eta as eta
		if ((in.Context.RayFlags & RF_Camera) && out.Type == MST_SpecularTransmission)
			out.Weight /= eta * eta;
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
		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatSpectralNode> mTransmission;
	const std::shared_ptr<FloatSpectralNode> mIOR;
};

// System of function which probably could be simplified with template meta programming
template <bool HasTransmissionColor, bool IsThin, bool SpectralVarying>
static std::shared_ptr<IMaterial> createMaterial1(uint32 id, const SceneLoadContext& ctx)
{
	return std::make_shared<DielectricMaterial<HasTransmissionColor, IsThin, SpectralVarying>>(
		id,
		ctx.lookupSpectralNode("specularity", 1),
		HasTransmissionColor ? ctx.lookupSpectralNode("transmission", 1) : nullptr,
		ctx.lookupSpectralNode("index", 1.55f));
}

template <bool HasTransmissionColor, bool IsThin>
static std::shared_ptr<IMaterial> createMaterial2(uint32 id, const SceneLoadContext& ctx)
{
	const bool spectralVarying = ctx.parameters().getBool("spectral_varying", true);
	if (spectralVarying)
		return createMaterial1<HasTransmissionColor, IsThin, true>(id, ctx);
	else
		return createMaterial1<HasTransmissionColor, IsThin, false>(id, ctx);
}

template <bool HasTransmissionColor>
static std::shared_ptr<IMaterial> createMaterial3(uint32 id, const SceneLoadContext& ctx)
{
	const bool isThin = ctx.parameters().getBool("thin", false);
	if (isThin)
		return createMaterial2<HasTransmissionColor, true>(id, ctx);
	else
		return createMaterial2<HasTransmissionColor, false>(id, ctx);
}

static std::shared_ptr<IMaterial> createMaterial4(uint32 id, const SceneLoadContext& ctx)
{
	const bool hasTransmission = ctx.parameters().hasParameter("transmission");
	if (hasTransmission)
		return createMaterial3<true>(id, ctx);
	else
		return createMaterial3<false>(id, ctx);
}

class DielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		// Construct rough dielectric instead
		/*if (ctx.parameters().hasParameter("roughness")
			|| ctx.parameters().hasParameter("roughness_x")
			|| ctx.parameters().hasParameter("roughness_y"))
			return ctx.yieldToMaterial(id, "roughdielectric", ctx.parameters());*/

		return createMaterial4(id, ctx);
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