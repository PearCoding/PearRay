#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/MicrofacetReflection.h"
#include "math/MicrofacetTransmission.h"
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

// TODO: Thin
constexpr float AIR = 1.0002926f;
template <bool UseVNDF, bool IsAnisotropic>
struct RoughDielectricClosure {
	using Reflection   = MicrofacetReflection<IsAnisotropic, UseVNDF>;
	using Transmission = MicrofacetTransmission<IsAnisotropic, UseVNDF>;

	const float M1;
	const float M2;
	const SpectralBlob SpecularityFactor;
	const SpectralBlob TransmissionFactor;
	const SpectralBlob IOR;

	inline RoughDielectricClosure(float M1, float M2, const SpectralBlob& specularity, const SpectralBlob& transmission, const SpectralBlob& ior)
		: M1(M1)
		, M2(M2)
		, SpecularityFactor(specularity)
		, TransmissionFactor(transmission)
		, IOR(ior)
	{
	}

	// Special constructor to be used when only the pdf is evaluated
	inline RoughDielectricClosure(float M1, float M2, const SpectralBlob& ior)
		: M1(M1)
		, M2(M2)
		, SpecularityFactor(SpectralBlob::Ones())
		, TransmissionFactor(SpectralBlob::Ones())
		, IOR(ior)
	{
	}

	inline bool isDelta() const { return Reflection(M1, M2).isDelta(); }

	inline SpectralBlob eval(const ShadingVector& V, const ShadingVector& L, bool isLightPath) const
	{
		if (V.sameHemisphere(L)) {
			const auto reflection = Reflection(M1, M2);

			SpectralBlob rWeight;
			PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				rWeight[i] = reflection.evalDielectric(V, L, AIR, IOR[i]);

			return rWeight * SpecularityFactor;
		} else {
			SpectralBlob tWeight;
			PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const auto transmission = Transmission(M1, M2, AIR, IOR[i]);
				tWeight[i]				= transmission.evalDielectric(V, L, isLightPath);
			}
			return tWeight * TransmissionFactor;
		}
	}

	inline SpectralBlob pdf(const ShadingVector& V, const ShadingVector& L) const
	{
		// Calculate Fresnel term
		SpectralBlob F;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			F[i] = Fresnel::dielectric(V.cosTheta(), AIR, IOR[i]);

		if (V.sameHemisphere(L)) {
			const auto reflection = Reflection(M1, M2);

			SpectralBlob rPdf;
			PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				rPdf[i] = reflection.pdf(L, V);

			return F * rPdf;
		} else {
			SpectralBlob tPdf;
			PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const auto transmission = Transmission(M1, M2, AIR, IOR[i]);
				tPdf[i]					= transmission.pdf(V, L);
			}
			return (1 - F) * tPdf;
		}
	}

	inline Vector3f sample(Random& rnd, const ShadingVector& V) const
	{
		// Calculate Fresnel term
		const float F = Fresnel::dielectric(V.cosTheta(), AIR, IOR[0]); /* Hero Wavelength */

		// Breach out into two cases
		if (rnd.getFloat() <= F) {
			const auto reflection = Reflection(M1, M2);
			return reflection.sample(rnd.get2D(), V);
		} else {
			const auto refraction = Transmission(M1, M2, AIR, IOR[0] /* Hero Wavelength */);
			return refraction.sample(rnd.get2D(), V);
		}
	}
};

template <bool HasTransmissionColor, bool UseVNDF, bool IsAnisotropic>
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
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~RoughDielectricMaterial() = default;

	inline RoughDielectricClosure<UseVNDF, IsAnisotropic> getClosure(const ShadingContext& sctx) const
	{
		const float m1			= mRoughnessX->eval(sctx);
		const SpectralBlob spec = mSpecularity->eval(sctx);

		return RoughDielectricClosure<UseVNDF, IsAnisotropic>(m1,
															  IsAnisotropic ? mRoughnessY->eval(sctx) : m1,
															  spec,
															  HasTransmissionColor ? mTransmission->eval(sctx) : spec,
															  mIOR->eval(sctx));
	}

	inline RoughDielectricClosure<UseVNDF, IsAnisotropic> getClosurePdf(const ShadingContext& sctx) const
	{
		const float m1 = mRoughnessX->eval(sctx);
		return RoughDielectricClosure<UseVNDF, IsAnisotropic>(m1,
															  IsAnisotropic ? mRoughnessY->eval(sctx) : m1,
															  mIOR->eval(sctx));
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = getClosure(in.ShadingContext);
		if (closure.isDelta()) { // Reject
			out.PDF_S  = 0.0f;
			out.Weight = SpectralBlob::Zero();
			out.Flags  = MaterialScatter::DeltaDistribution;
			return;
		}

		out.Weight = closure.eval(in.Context.V, in.Context.L, in.Context.RayFlags & RayFlag::Light);
		out.PDF_S  = closure.pdf(in.Context.V, in.Context.L);

		// Determine type
		if (in.Context.V.sameHemisphere(in.Context.L))
			out.Type = MaterialScatteringType::SpecularReflection;
		else
			out.Type = MaterialScatteringType::SpecularTransmission;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = getClosurePdf(in.ShadingContext);
		if (closure.isDelta()) { // Reject
			out.PDF_S = 0.0f;
			out.Flags = MaterialScatter::DeltaDistribution;
			return;
		}

		out.PDF_S = closure.pdf(in.Context.V, in.Context.L);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		const auto closure = getClosure(in.ShadingContext);

		out.L = closure.sample(in.RND, in.Context.V);

		// Set flags
		if (closure.isDelta())
			out.Flags |= MaterialScatter::DeltaDistribution;

		if (PR_UNLIKELY(out.L.isZero())) {
			out = MaterialSampleOutput::Reject(MaterialScatteringType::SpecularReflection, out.Flags);
			return;
		}

		out.Weight = closure.eval(in.Context.V, out.L, in.Context.RayFlags & RayFlag::Light);
		out.PDF_S  = closure.pdf(in.Context.V, out.L);

		// If we handle a delta case, make sure the outgoing pdf will be 1
		if (closure.isDelta() && out.PDF_S[0] > PR_EPSILON) {
			out.Weight /= out.PDF_S[0];
			out.PDF_S = 1.0f;
		}

		// Set type
		if (in.Context.V.sameHemisphere(out.L))
			out.Type = MaterialScatteringType::SpecularReflection;
		else
			out.Type = MaterialScatteringType::SpecularTransmission;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <RoughDielectricMaterial>:" << std::endl
			   << "    Specularity:     " << mSpecularity->dumpInformation() << std::endl
			   << "    Transmission:    " << mTransmission->dumpInformation() << std::endl
			   << "    IOR:             " << mIOR->dumpInformation() << std::endl
			   << "    RoughnessX:      " << mRoughnessX->dumpInformation() << std::endl
			   << "    RoughnessY:      " << mRoughnessY->dumpInformation() << std::endl
			   << "    VNDF:            " << (UseVNDF ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mSpecularity;
	const std::shared_ptr<FloatSpectralNode> mTransmission;
	const std::shared_ptr<FloatSpectralNode> mIOR;
	const std::shared_ptr<FloatScalarNode> mRoughnessX;
	const std::shared_ptr<FloatScalarNode> mRoughnessY;
};

// System of function which probably could be simplified with template meta programming
template <bool HasTransmissionColor, bool UseVNDF>
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
		return std::make_shared<RoughDielectricMaterial<HasTransmissionColor, UseVNDF, false>>(spec, trans, index, rx, ry);
	else
		return std::make_shared<RoughDielectricMaterial<HasTransmissionColor, UseVNDF, true>>(spec, trans, index, rx, ry);
}

template <bool HasTransmissionColor>
static std::shared_ptr<IMaterial> createMaterial2(const SceneLoadContext& ctx)
{
	const bool use_vndf = ctx.parameters().getBool("vndf", true);

	if (use_vndf)
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

class RoughDielectricMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return createMaterial3(ctx);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "roughglass", "roughdielectric", "rough_glass", "rough_dielectric" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Rough Dielectric BSDF", "A rough dielectric BSDF")
			.Identifiers(getNames())
			.Inputs()
			.BeginBlock("Roughness", PluginParamDescBlockOp::OneOf)
			.ScalarNode("roughness", "Isotropic roughness", 0.0f)
			.BeginBlock("")
			.ScalarNode("roughness_x", "Anisotropic x roughness", 0.0f)
			.ScalarNode("roughness_y", "Anisotropic y roughness", 0.0f)
			.EndBlock()
			.EndBlock()
			.SpectralNodeV({ "index", "eta", "ior" }, "Index of refraction", 1.55f)
			.SpectralNode("specularity", "Tint", 1.0f)
			.SpectralNode("transmission", "Tint", 1.0f)
			.Bool("vndf", "Use sampling method based on the viewing normal", true)
			.Specification()
			.get();
	}
	
	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RoughDielectricMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)