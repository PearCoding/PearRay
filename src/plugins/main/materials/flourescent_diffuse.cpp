#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {
struct FlourescentDiffuseClosure {
	const SpectralBlob Absorption;
	const SpectralBlob Emission;
	const SpectralBlob AEFactor;
	const SpectralBlob Albedo;
	const float Concentration;

	FlourescentDiffuseClosure(const SpectralBlob& absorption,
							  const SpectralBlob& emission,
							  const SpectralBlob& aeFactor,
							  const SpectralBlob& albedo,
							  float concentration)
		: Absorption(absorption)
		, Emission(emission)
		, AEFactor(aeFactor)
		, Albedo(albedo)
		, Concentration(concentration)
	{
	}

	inline SpectralBlob evalFlourescent(const MaterialEvalContext& ctx) const
	{
		return Concentration * Absorption * AEFactor * Emission * ctx.L.absCosTheta() * PR_INV_PI;
	}

	inline SpectralBlob evalNonFlourescent(const MaterialEvalContext& ctx) const
	{
		return (1 - Concentration * Absorption) * Albedo * ctx.L.absCosTheta() * PR_INV_PI;
	}

	inline SpectralBlob eval(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		const SpectralBlob delta = 1 - (ctx.WavelengthNM - ctx.FlourescentWavelengthNM).cwiseAbs();
		return evalFlourescent(ctx) + delta * evalNonFlourescent(ctx);
	}

	inline SpectralBlob pdfDiffuse(const MaterialEvalContext& ctx) const
	{
		return SpectralBlob(Sampling::cos_hemi_pdf(ctx.L.absCosTheta()));
	}

	inline SpectralBlob pdfSelectFlourescent(const MaterialSampleContext& ctx) const
	{
		if (ctx.RayFlags & RayFlag::Light) {
			const SpectralBlob A = Concentration * Absorption * AEFactor;
			const SpectralBlob B = (1 - Concentration * Absorption) * Albedo;
			return A / (A + B);
		} else {
			const float IntegralAlbedo = Absorption.sum(); // TODO: Actually for all wavelengths!
			const SpectralBlob A	   = Concentration * IntegralAlbedo * AEFactor * Emission;
			const SpectralBlob B	   = (1 - Concentration * Absorption) * Albedo;
			return A / (A + B);
		}
	}

	inline SpectralBlob pdf(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		/*const SpectralBlob delta = 1 - (ctx.WavelengthNM - ctx.FlourescentWavelengthNM).cwiseAbs();
		const SpectralBlob flou	 = pdfSelectFlourescent(ctx);

		return flou * pdfDiffuse(ctx) + delta * (1-flou) * pdfDiffuse(ctx);*/
		return pdfDiffuse(ctx);
	}

	inline Vector3f sampleDiffuse(Random& rnd, const MaterialSampleContext& ctx) const
	{
		const bool flip	 = ctx.V.cosTheta() < 0;
		const Vector3f L = Sampling::cos_hemi(rnd.getFloat(), rnd.getFloat());
		return flip ? -L : L;
	}

	inline SpectralBlob sampleWavelength(Random& rnd, const MaterialSampleContext& ctx) const
	{
		return ctx.WavelengthNM; // TODO
	}

	inline std::pair<Vector3f, SpectralBlob> sampleFlourescent(Random& rnd, const MaterialSampleContext& ctx) const
	{
		return { sampleDiffuse(rnd, ctx), sampleWavelength(rnd, ctx) };
	}

	inline std::pair<Vector3f, SpectralBlob> sampleNonFlourescent(Random& rnd, const MaterialSampleContext& ctx) const
	{
		return { sampleDiffuse(rnd, ctx), ctx.WavelengthNM };
	}

	inline std::pair<Vector3f, SpectralBlob> sample(Random& rnd, const MaterialSampleContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON))
			return { Vector3f::Zero(), ctx.WavelengthNM };

		const SpectralBlob prob = pdfSelectFlourescent(ctx);

		if (rnd.getFloat() < prob[0])
			return sampleFlourescent(rnd, ctx);
		else
			return sampleNonFlourescent(rnd, ctx);
	}

	inline bool isFlourescent(const MaterialEvalContext& ctx) const
	{
		return (ctx.WavelengthNM != ctx.FlourescentWavelengthNM).any();
	}
};

/* Based on Jung, A., J. Hanika, Steve Marschner and C. Dachsbacher. “A Simple Diffuse Fluorescent BBRRDF Model.” MAM@EGSR (2018). */
class FlourescentDiffuse : public IMaterial {
public:
	FlourescentDiffuse(const std::shared_ptr<FloatSpectralNode>& absorption, const std::shared_ptr<FloatSpectralNode>& emission,
					   const std::shared_ptr<FloatSpectralNode>& absorpedEmissionFactor, const std::shared_ptr<FloatSpectralNode>& albedo,
					   const std::shared_ptr<FloatScalarNode>& concentration)
		: IMaterial()
		, mAbsorption(absorption)
		, mEmission(emission)
		, mAbsorpedEmissionFactor(absorpedEmissionFactor) // Q
		, mAlbedo(albedo)								  // Non-flourescent property
		, mConcentration(concentration)
	{
	}

	virtual ~FlourescentDiffuse() = default;

	inline FlourescentDiffuseClosure createClosure(const ShadingContext& sctx) const
	{
		return FlourescentDiffuseClosure(mAbsorption->eval(sctx), mEmission->eval(sctx),
										 mAbsorpedEmissionFactor->eval(sctx), mAlbedo->eval(sctx),
										 mConcentration->eval(sctx));
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = createClosure(in.ShadingContext);

		out.Weight = closure.eval(in.Context);
		out.PDF_S  = closure.pdf(in.Context);
		out.Type   = MaterialScatteringType::DiffuseReflection;

		if (closure.isFlourescent(in.Context))
			out.Flags = MaterialSampleFlag::Flourescent;
		else
			out.Flags = 0;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		const auto closure = createClosure(in.ShadingContext);

		out.PDF_S = closure.pdf(in.Context);

		if (closure.isFlourescent(in.Context))
			out.Flags = MaterialSampleFlag::Flourescent;
		else
			out.Flags = 0;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure			= createClosure(in.ShadingContext);
		const auto lwp				= closure.sample(in.RND, in.Context);
		out.L						= lwp.first;
		out.FlourescentWavelengthNM = lwp.second;

		auto extendedCtx					= in.Context.expand(out.L);
		extendedCtx.FlourescentWavelengthNM = out.FlourescentWavelengthNM;

		out.Weight = closure.eval(extendedCtx);
		out.PDF_S  = closure.pdf(extendedCtx);

		out.Type = MaterialScatteringType::DiffuseReflection;
		if (closure.isFlourescent(extendedCtx))
			out.Flags = MaterialSampleFlag::Flourescent;
		else
			out.Flags = 0;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <FlourescentDiffuse>:" << std::endl
			   << "    Absorption:    " << mAbsorption->dumpInformation() << std::endl
			   << "    Emission:      " << mEmission->dumpInformation() << std::endl
			   << "    AEFactor:      " << mAbsorpedEmissionFactor->dumpInformation() << std::endl
			   << "    Albedo:        " << mAlbedo->dumpInformation() << std::endl
			   << "    Concentration: " << mConcentration->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mAbsorption;
	const std::shared_ptr<FloatSpectralNode> mEmission;
	const std::shared_ptr<FloatSpectralNode> mAbsorpedEmissionFactor;
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
	const std::shared_ptr<FloatScalarNode> mConcentration;
};

inline constexpr float stokesShiftDelta(float nm, float nm_shift) { return 1 / nm - 1 / (nm + nm_shift); }

class FlourescentDiffuseMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const auto absorption	 = ctx.lookupSpectralNode("absorption", 0.5f);
		const auto emission		 = ctx.lookupSpectralNode("emission", 1);
		const auto aefactor		 = ctx.lookupSpectralNode("aefactor", 1); // All absorped wavelengths will be emitted
		const auto albedo		 = ctx.lookupSpectralNode({ "albedo", "base", "diffuse" }, 1);
		const auto concentration = ctx.lookupScalarNode("concentration", 1);

		return std::make_shared<FlourescentDiffuse>(absorption, emission, aefactor, albedo, concentration);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "flourescent", "flourescent_diffuse" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Flourescent Diffuse BSDF", "A perfect diffuse BSDF with flourescent properties")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode( "absoprtion" , "Amount of light which is absorpted and might be used for flourescent", 0.5f)
			.SpectralNode( "emission" , "Amount of light which is emitted as flourescent light", 1.0f)
			.SpectralNode( "aefactor" , "Amount of absorped light which is emitted as flourescent light", 1.0f)
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected for non-flourescent part", 1.0f)
			.ScalarNode("concentration", "How much the flourescent part is affected", 1.0f)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FlourescentDiffuseMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)