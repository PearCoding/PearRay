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

// TODO: Better make it a parameter
constexpr float AIR = 1.0002926f;
template <bool HasTransmissionColor, bool IsThin>
class DielectricMaterial : public IMaterial {
public:
	DielectricMaterial(const std::shared_ptr<FloatSpectralNode>& spec,
					   const std::shared_ptr<FloatSpectralNode>& trans,
					   const std::shared_ptr<FloatSpectralNode>& ior)
		: IMaterial()
		, mSpecularity(spec)
		, mTransmission(trans)
		, mIOR(ior)
	{
	}

	virtual ~DielectricMaterial() = default;

	MaterialFlags flags() const override { return MaterialFlag::OnlyDeltaDistribution; }

	void eval(const MaterialEvalInput&, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S  = 0.0f;
		out.Type   = MaterialScatteringType::SpecularTransmission;
		out.Weight = SpectralBlob::Zero();
		out.Flags  = MaterialScatter::DeltaDistribution;
	}

	void pdf(const MaterialEvalInput&, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		PR_ASSERT(false, "Delta distribution materials should not be evaluated");

		out.PDF_S = 0.0f;
		out.Flags = MaterialScatter::DeltaDistribution;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.PDF_S = 1;

		const SpectralBlob n2 = mIOR->eval(in.ShadingContext);
		float F				  = Fresnel::dielectric(in.Context.V.cosTheta(), AIR, n2[0]);

		if constexpr (IsThin) {
			// Account for scattering between interfaces
			if (F < 1.0f)
				F += (1 - F) * F / (F + 1);
		}

		const SpectralBlob rWeight = mSpecularity->eval(in.ShadingContext);

		// Branch out
		if (in.RND.getFloat() <= F) {
			out.Type   = MaterialScatteringType::SpecularReflection;
			out.L	   = Scattering::reflect(in.Context.V);
			out.Weight = rWeight; // To make sure the pdf is 1, we do not apply the fresnel term to the outgoing weight
		} else {
			SpectralBlob tWeight;
			if constexpr (HasTransmissionColor)
				tWeight = mTransmission->eval(in.ShadingContext);
			else
				tWeight = rWeight;

			if constexpr (IsThin) {
				out.Type   = MaterialScatteringType::SpecularTransmission;
				out.L	   = -in.Context.V; // Passthrough
				out.Weight = tWeight;
			} else {
				// Only rays from lights are weighted by this factor
				// as radiance flows in the opposite direction
				if (in.Context.RayFlags & RayFlag::Light) {
					const float eta = in.Context.V.isPositiveHemisphere() ? AIR / n2[0] : n2[0] / AIR;
					tWeight *= eta * eta;
				}

				out.L = Scattering::refract(AIR / n2[0], in.Context.V);

				if (out.L.sameHemisphere(in.Context.V)) { // TOTAL REFLECTION
					out.Type   = MaterialScatteringType::SpecularReflection;
					out.Weight = rWeight;
				} else {
					out.Type   = MaterialScatteringType::SpecularTransmission;
					out.Weight = tWeight;
				}
			}
		}

		out.Flags = MaterialScatter::DeltaDistribution;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <DielectricMaterial>:" << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl
			   << "    Transmission:    " << mTransmission->dumpInformation() << std::endl
			   << "    IOR:             " << mIOR->dumpInformation() << std::endl
			   << "    IsThin:          " << (IsThin ? "true" : "false") << std::endl;
		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatSpectralNode> mTransmission;
	const std::shared_ptr<FloatSpectralNode> mIOR;
};

// System of function which probably could be simplified with template meta programming
template <bool HasTransmissionColor, bool IsThin>
static std::shared_ptr<IMaterial> createMaterial1(const SceneLoadContext& ctx)
{
	const auto specular = ctx.lookupSpectralNode("specularity", 1);
	return std::make_shared<DielectricMaterial<HasTransmissionColor, IsThin>>(
		specular,
		HasTransmissionColor ? ctx.lookupSpectralNode("transmission", 1) : specular,
		ctx.lookupSpectralNode("index", 1.55f));
}

template <bool HasTransmissionColor>
static std::shared_ptr<IMaterial> createMaterial2(const SceneLoadContext& ctx)
{
	const bool isThin = ctx.parameters().getBool("thin", false);
	if (isThin)
		return createMaterial1<HasTransmissionColor, true>(ctx);
	else
		return createMaterial1<HasTransmissionColor, false>(ctx);
}

static std::shared_ptr<IMaterial> createMaterial3(const SceneLoadContext& ctx)
{
	const bool hasTransmission = ctx.parameters().hasParameter("transmission");
	if (hasTransmission)
		return createMaterial2<true>(ctx);
	else
		return createMaterial2<false>(ctx);
}

class DielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		// Construct rough dielectric instead
		if (ctx.parameters().hasParameter("roughness")
			|| ctx.parameters().hasParameter("roughness_x")
			|| ctx.parameters().hasParameter("roughness_y"))
			return ctx.loadMaterial("roughdielectric", ctx.parameters());

		return createMaterial3(ctx);
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