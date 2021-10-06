#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Distribution1D.h"
#include "math/Sampling.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "shader/NodeUtils.h"

#include <cmath>
#include <sstream>

namespace PR {
static inline bool checkIfFluorescent(const SpectralBlob& aWvl, const SpectralBlob& bWvl) { return ((aWvl - bWvl).cwiseAbs() >= 0.5f).any(); }

struct FluorescentDiffuseClosure {
	const SpectralBlob AbsorptionIn;
	const SpectralBlob AbsorptionOut; // Used in camera paths pdf
	const SpectralBlob EmissionOut;
	const SpectralBlob AlbedoIn;
	const SpectralBlob AlbedoOut; // Used in camera paths pdf
	const float AEFactor;
	const float Concentration;
	const SpectralRange Range;
	const Distribution1D& Distribution;
	const float AbsorptionIntegral;

	FluorescentDiffuseClosure(const SpectralBlob& absorptionIn,
							  const SpectralBlob& absorptionOut,
							  const SpectralBlob& emission,
							  const SpectralBlob& albedoIn,
							  const SpectralBlob& albedoOut,
							  float aeFactor,
							  float concentration,
							  const SpectralRange& range,
							  const Distribution1D& distr,
							  float absorptionIntegral)
		: AbsorptionIn(absorptionIn)
		, AbsorptionOut(absorptionOut)
		, EmissionOut(emission)
		, AlbedoIn(albedoIn)
		, AlbedoOut(albedoOut)
		, AEFactor(aeFactor)
		, Concentration(concentration)
		, Range(range)
		, Distribution(distr)
		, AbsorptionIntegral(absorptionIntegral)
	{
	}

	inline SpectralBlob nonFlourescency(const MaterialEvalContext& ctx) const
	{
		const SpectralBlob delta = (ctx.WavelengthNM - ctx.FluorescentWavelengthNM).cwiseAbs();
		return (delta < 1).select(delta, 0);
	}

	inline SpectralBlob evalFluorescent(const MaterialEvalContext& ctx) const
	{
		return (Concentration * AEFactor) * AbsorptionIn * EmissionOut * ctx.L.absCosTheta() * PR_INV_PI;
	}

	inline SpectralBlob evalNonFluorescent(const MaterialEvalContext& ctx) const
	{
		return (1 - Concentration * AbsorptionIn) * AlbedoIn * ctx.L.absCosTheta() * PR_INV_PI;
	}

	inline SpectralBlob eval(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		return evalFluorescent(ctx) + nonFlourescency(ctx) * evalNonFluorescent(ctx);
	}

	inline SpectralBlob pdfWavelengthFluorescent(const MaterialEvalContext& ctx) const
	{
		SpectralBlob blob;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			blob[i] = Distribution.continuousPdf((ctx.FluorescentWavelengthNM[i] - Range.Start) / Range.span());
		return blob;
	}

	inline SpectralBlob pdfDiffuse(const MaterialEvalContext& ctx) const
	{
		return SpectralBlob(Sampling::cos_hemi_pdf(ctx.L.absCosTheta()));
	}

	inline SpectralBlob pdfSelectFluorescent(const MaterialSampleContext& ctx) const
	{
		if (ctx.RayFlags & RayFlag::Light) {
			// Every term givin in respect to the "light" wavelength
			const SpectralBlob A = Concentration * AbsorptionIn * AEFactor;
			const SpectralBlob B = (1 - Concentration * AbsorptionIn) * AlbedoIn;
			const SpectralBlob r = A / (A + B);
			return r.isFinite().select(r, 0);
		} else {
			// Every term givin in respect to the "scatter" wavelength
			const SpectralBlob A = Concentration * AbsorptionIntegral * AEFactor * EmissionOut;
			const SpectralBlob B = (1 - Concentration * AbsorptionOut) * AlbedoOut;
			const SpectralBlob r = A / (A + B);
			return r.isFinite().select(r, 0);
		}
	}

	inline SpectralBlob pdfFluorescent(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		const SpectralBlob flou = pdfSelectFluorescent(ctx);
		PR_ASSERT((flou >= 0).all() && (flou <= 1).all(), "Expected bounded selection propability");
		return pdfDiffuse(ctx) * flou * pdfWavelengthFluorescent(ctx);
	}

	inline SpectralBlob pdfNonFluorescent(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		const SpectralBlob flou = pdfSelectFluorescent(ctx);
		PR_ASSERT((flou >= 0).all() && (flou <= 1).all(), "Expected bounded selection propability");
		return pdfDiffuse(ctx) * (1 - flou);
	}

	inline SpectralBlob pdf(const MaterialEvalContext& ctx) const
	{
		return pdfFluorescent(ctx) + nonFlourescency(ctx) * pdfNonFluorescent(ctx);
	}

	inline Vector3f sampleDiffuse(Random& rnd, const MaterialSampleContext& ctx) const
	{
		const bool flip	 = ctx.V.cosTheta() < 0;
		const Vector3f L = Sampling::cos_hemi(rnd.getFloat(), rnd.getFloat());
		return flip ? -L : L;
	}

	inline static std::pair<SpectralBlob, float> sampleWavelength(Random& rnd,
																  const SpectralRange& range,
																  const Distribution1D& distr)
	{
		float pdf	  = 0;
		const float w = range.span();

		SpectralBlob b;
		b[0] = range.Start + distr.sampleContinuous(rnd.getFloat(), pdf) * w;
		PR_OPT_LOOP
		for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			b[i] = std::fmod(b[0] - range.Start + i * w, w) + range.Start;

		return { b, pdf };
	}

	inline std::pair<Vector3f, bool> sample(Random& rnd, const MaterialSampleContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON))
			return { Vector3f::Zero(), false };

		const SpectralBlob prob = pdfSelectFluorescent(ctx);

		return { sampleDiffuse(rnd, ctx), (rnd.getFloat() < prob[0]) };
	}

	inline bool isFluorescent(const MaterialEvalContext& ctx) const
	{
		return checkIfFluorescent(ctx.WavelengthNM, ctx.FluorescentWavelengthNM);
	}

	inline MaterialSampleFlags sampleFlags(const MaterialEvalContext& ctx) const
	{
		return isFluorescent(ctx) ? MaterialSampleFlag::Fluorescent : (MaterialSampleFlags)0;
	}
};

/* Based on Jung, A., J. Hanika, Steve Marschner and C. Dachsbacher. "A Simple Diffuse Fluorescent BBRRDF Model." MAM@EGSR (2018). */
class FluorescentDiffuse : public IMaterial {
public:
	FluorescentDiffuse(const std::shared_ptr<FloatSpectralNode>& absorption, const std::shared_ptr<FloatSpectralNode>& emission,
					   const std::shared_ptr<FloatSpectralNode>& albedo,
					   float absorpedEmissionFactor, float concentration, float scale)
		: IMaterial()
		, mAbsorption(absorption)
		, mEmission(emission)
		, mAlbedo(albedo)								  // Non-fluorescent property
		, mAbsorbedEmissionFactor(absorpedEmissionFactor) // Q
		, mConcentration(concentration)
		, mScale(scale)
		, mDistributionEmission(400)
		, mDistributionAbsorption(400)
		, mEmissionIntegral(0)
		, mAbsorptionIntegral(0)
	{
		buildEmissionDistribution();
		buildAbsorptionDistribution();
	}

	virtual ~FluorescentDiffuse() = default;

	inline FluorescentDiffuseClosure createClosure(const ShadingContext& sctx,
												   const SpectralBlob& fluorescentWavelengthNM,
												   bool light) const
	{
		const SpectralRange range	= light ? mEmission->spectralRange() : mAbsorption->spectralRange();
		const Distribution1D& distr = light ? mDistributionEmission : mDistributionAbsorption;

		PR_ASSERT(!range.hasUnbounded(), "Expected wavelength bounded emission data"); // TODO: Check user input!!!!

		if (checkIfFluorescent(sctx.WavelengthNM, fluorescentWavelengthNM)) {
			ShadingContext sctx2 = sctx;
			sctx2.WavelengthNM	 = fluorescentWavelengthNM;

			// Make sure the light flow direction is taken into account!
			// If we trace a camera path, the wavelength would be the scatter/out side and fluorescent wavelength the light/in side
			// Vice versa for light paths
			const ShadingContext& sctxIn  = light ? sctx : sctx2;
			const ShadingContext& sctxOut = light ? sctx2 : sctx;

			return FluorescentDiffuseClosure(mAbsorption->eval(sctxIn), mAbsorption->eval(sctxOut),
											 mEmission->eval(sctxOut) / mEmissionIntegral /* normalize it */,
											 mAlbedo->eval(sctxIn), mAlbedo->eval(sctxOut),
											 mAbsorbedEmissionFactor, mConcentration,
											 range, distr, mAbsorptionIntegral);
		} else {
			const SpectralBlob absorption = mAbsorption->eval(sctx);
			const SpectralBlob albedo	  = mAlbedo->eval(sctx);

			return FluorescentDiffuseClosure(absorption, absorption,
											 mEmission->eval(sctx) / mEmissionIntegral /* normalize it */,
											 albedo, albedo,
											 mAbsorbedEmissionFactor, mConcentration,
											 range, distr, mAbsorptionIntegral);
		}
	}

	MaterialFlags flags() const override { return MaterialFlag::HasFluorescence; }

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = createClosure(in.ShadingContext, in.Context.FluorescentWavelengthNM, in.Context.RayFlags & RayFlag::Light);

		out.Weight = mScale * closure.eval(in.Context);
		out.PDF_S  = closure.pdf(in.Context);
		out.Type   = MaterialScatteringType::DiffuseReflection;
		out.Flags  = closure.sampleFlags(in.Context);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		const auto closure = createClosure(in.ShadingContext, in.Context.FluorescentWavelengthNM, in.Context.RayFlags & RayFlag::Light);

		out.PDF_S = closure.pdf(in.Context);
		out.Flags = closure.sampleFlags(in.Context);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const bool isLight = in.Context.RayFlags & RayFlag::Light;
		const auto sampleW = isLight
								 ? FluorescentDiffuseClosure::sampleWavelength(in.RND, mEmission->spectralRange(), mDistributionEmission)
								 : FluorescentDiffuseClosure::sampleWavelength(in.RND, mAbsorption->spectralRange(), mDistributionAbsorption);

		const auto closure			= createClosure(in.ShadingContext, sampleW.first, isLight);
		const auto lwp				= closure.sample(in.RND, in.Context);
		out.L						= in.Context.V.makeSameHemisphere(lwp.first);
		out.FluorescentWavelengthNM = lwp.second ? sampleW.first : in.Context.WavelengthNM;

		auto extendedCtx					= in.Context.expandLocal(out.L);
		extendedCtx.FluorescentWavelengthNM = out.FluorescentWavelengthNM;

		out.IntegralWeight = mScale * closure.eval(extendedCtx);
		out.PDF_S		   = lwp.second ? closure.pdfFluorescent(extendedCtx) : closure.pdfNonFluorescent(extendedCtx);
		out.IntegralWeight /= out.PDF_S[0];

		out.Type  = MaterialScatteringType::DiffuseReflection;
		out.Flags = closure.sampleFlags(extendedCtx);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <FluorescentDiffuse>:" << std::endl
			   << "    Absorption:    " << mAbsorption->dumpInformation() << std::endl
			   << "    Emission:      " << mEmission->dumpInformation() << std::endl
			   << "    Albedo:        " << mAlbedo->dumpInformation() << std::endl
			   << "    AEFactor:      " << mAbsorbedEmissionFactor << std::endl
			   << "    Concentration: " << mConcentration << std::endl
			   << "    Scale:         " << mScale << std::endl;

		return stream.str();
	}

private:
	// Actually, it would be better to sample the whole composition of emission etc
	void buildEmissionDistribution()
	{
		const SpectralRange range = mEmission->spectralRange();
		mDistributionEmission.generate([&](size_t wvl_i) {
			const float wvl = range.Start + range.span() * wvl_i / (float)mDistributionEmission.numberOfValues();
			return NodeUtils::average(SpectralBlob(wvl), mEmission.get())[0];
		},
									   &mEmissionIntegral);
	}

	void buildAbsorptionDistribution()
	{
		const SpectralRange range = mAbsorption->spectralRange();
		mDistributionAbsorption.generate([&](size_t wvl_i) {
			const float wvl = range.Start + range.span() * wvl_i / (float)mDistributionAbsorption.numberOfValues();
			return NodeUtils::average(SpectralBlob(wvl), mAbsorption.get())[0];
		},
										 &mAbsorptionIntegral);
	}

	const std::shared_ptr<FloatSpectralNode> mAbsorption;
	const std::shared_ptr<FloatSpectralNode> mEmission;
	const std::shared_ptr<FloatSpectralNode> mAlbedo;
	const float mAbsorbedEmissionFactor;
	const float mConcentration;
	const float mScale;

	Distribution1D mDistributionEmission;
	Distribution1D mDistributionAbsorption;
	float mEmissionIntegral;
	float mAbsorptionIntegral;
};

class FluorescentDiffuseMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const auto absorption = ctx.lookupSpectralNode("absorption", 0.5f);
		const auto emission	  = ctx.lookupSpectralNode("emission", 1);
		const auto albedo	  = ctx.lookupSpectralNode({ "albedo", "base", "diffuse" }, 1);

		const auto aefactor		 = ctx.parameters().getNumber("aefactor", 1); // All absorbed wavelengths will be emitted
		const auto concentration = ctx.parameters().getNumber("concentration", 1);
		const auto scale		 = ctx.parameters().getNumber("scale", 1);

		return std::make_shared<FluorescentDiffuse>(absorption, emission, albedo, aefactor, concentration, scale);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "fluorescent", "fluorescent_diffuse" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Fluorescent Diffuse BSDF", "A perfect diffuse BSDF with fluorescent properties")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode("absorption", "Amount of light which is absorbed and might be used for fluorescent", 0.5f)
			.SpectralNode("emission", "Amount of light which is emitted as fluorescent light", 1.0f)
			.SpectralNodeV({ "albedo", "base", "diffuse" }, "Amount of light which is reflected for non-fluorescent part", 1.0f)
			.Number01("aefactor", "Amount of absorbed light which is emitted as fluorescent light", 1.0f)
			.Number01("concentration", "How much the fluorescent part is affected", 1.0f)
			.Number("scale", "Artificial scale factor to scale up the bsdf eval value", 1.0f)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::FluorescentDiffuseMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)