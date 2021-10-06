#include "Environment.h"
#include "Random.h"
#include "SceneLoadContext.h"
#include "light/LightSampler.h"
#include "renderer/RenderContext.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"
#include "spectral/RGBConverter.h"

#include "Logger.h"
#include "Standard.h"

#include <mutex>

namespace PR {
struct SPDContext {
	Distribution1D Distribution;
	std::vector<Distribution1D> LightDistributions;

	inline SPDContext(size_t bins)
		: Distribution(bins)
	{
	}
};

class CMISSPDSpectralMapper : public ISpectralMapper {
public:
	CMISSPDSpectralMapper(const SpectralRange& cameraRange, const SpectralRange& lightRange, const SPDContext& context)
		: ISpectralMapper()
		, mCameraRange(cameraRange)
		, mLightRange(lightRange)
		, mContext(context)
	{
	}

	virtual ~CMISSPDSpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			const float u				= in.RND.getFloat();
			const Distribution1D& distr = mContext.Distribution;

			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const float k		= std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
				out.WavelengthNM(i) = distr.sampleContinuous(k /*in.RND.getFloat()*/, out.PDF(i)) * mCameraRange.span() + mCameraRange.Start;
			}
		} else if (in.Purpose == SpectralSamplePurpose::Light) {
			const float u				= in.RND.getFloat();
			const Distribution1D& distr = mContext.LightDistributions[in.Light->lightID()];
			const SpectralRange range	= in.Light->spectralRange().bounded(mCameraRange);

			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const float k		= std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
				out.WavelengthNM(i) = distr.sampleContinuous(k, out.PDF(i)) * range.span() + range.Start;
			}
		} else {
			out.WavelengthNM = sampleStandardWavelength(in.RND.getFloat(), mLightRange, out.PDF);
		}
	}

	// Do not apply the jacobian of the mapping from [0, 1] to [start, end], as we do not divide by it
	SpectralBlob pdf(const SpectralEvalInput& in, const SpectralBlob& wavelength) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			const Distribution1D& distr = mContext.Distribution;

			SpectralBlob res;
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				res(i) = distr.continuousPdf((wavelength(i) - mCameraRange.Start) / mCameraRange.span());
			return res;
		} else if (in.Purpose == SpectralSamplePurpose::Light) {
			const Distribution1D& distr = mContext.LightDistributions[in.Light->lightID()];
			const SpectralRange range	= in.Light->spectralRange().bounded(mCameraRange);

			SpectralBlob res;
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				res(i) = distr.continuousPdf((wavelength(i) - range.Start) / range.span());

			return res;
		} else {
			// For connections and cases with not enough information, we fallback to pure random sampling
			return pdfStandardWavelength(wavelength, mLightRange);
		}
	}

private:
	const SpectralRange mCameraRange;
	const SpectralRange mLightRange;
	const SPDContext& mContext;
};

class HeroSPDSpectralMapper : public ISpectralMapper {
public:
	HeroSPDSpectralMapper(const SpectralRange& cameraRange, const SpectralRange& lightRange, const SPDContext& context)
		: ISpectralMapper()
		, mCameraRange(cameraRange)
		, mLightRange(lightRange)
		, mContext(context)
	{
	}

	virtual ~HeroSPDSpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			const float u				= in.RND.getFloat();
			const Distribution1D& distr = mContext.Distribution;

			float pdf		 = 0;
			const float hero = distr.sampleContinuous(u, pdf) * mCameraRange.span() + mCameraRange.Start;
			out.WavelengthNM = constructHeroWavelength(hero, mCameraRange);
			out.PDF			 = pdf;
		} else if (in.Purpose == SpectralSamplePurpose::Light) {
			const float u				= in.RND.getFloat();
			const Distribution1D& distr = mContext.LightDistributions[in.Light->lightID()];
			const SpectralRange range	= in.Light->spectralRange().bounded(mCameraRange);

			float pdf		 = 0;
			const float hero = distr.sampleContinuous(u, pdf) * range.span() + range.Start;
			out.WavelengthNM = constructHeroWavelength(hero, range);
			out.PDF			 = pdf;
		} else {
			// For connections and cases with not enough information, we fallback to pure random sampling
			out.WavelengthNM = sampleStandardWavelength(in.RND.getFloat(), mLightRange, out.PDF);
		}
	}

	// Do not apply the jacobian of the mapping from [0, 1] to [start, end], as we do not divide by it
	SpectralBlob pdf(const SpectralEvalInput& in, const SpectralBlob& wavelength) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			const Distribution1D& distr = mContext.Distribution;
			return SpectralBlob(distr.continuousPdf((wavelength(0) - mCameraRange.Start) / mCameraRange.span()));
		} else if (in.Purpose == SpectralSamplePurpose::Light) {
			const Distribution1D& distr = mContext.LightDistributions[in.Light->lightID()];
			const SpectralRange range	= in.Light->spectralRange().bounded(mCameraRange);
			return SpectralBlob(distr.continuousPdf((wavelength(0) - range.Start) / range.span()));
		} else {
			// For connections and cases with not enough information, we fallback to pure random sampling
			return pdfStandardWavelength(wavelength, mLightRange);
		}
	}

private:
	const SpectralRange mCameraRange;
	const SpectralRange mLightRange;
	const SPDContext& mContext;
};

enum class WeightingMethod {
	None = 0,
	CIE_Y,
	CIE_XYZ,
	SRGB,
};

struct SPDParameters {
	/* This one can be high quality, as we sample each single wavelength!*/
	uint32 NumberOfBins = PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START;
	// Number of bins for single light distributions
	uint32 NumberOfLightBins = (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) / 2;

	WeightingMethod Method		= WeightingMethod::CIE_XYZ;
	bool UseNormalizedLights	= true;
	bool EnsureCompleteSampling = true;
	uint32 SmoothIterations		= 0; // How many times to apply moving average?
	bool Verbose				= false;
	bool UseCMIS				= true;
};

static void movingAverage(std::vector<float>& inout)
{
	const std::vector<float> tmp = inout;
	for (size_t i = 0; i < tmp.size(); ++i) {
		const float prevM = (i == 0) ? tmp.front() : tmp[i - 1];
		const float nextM = (i == tmp.size() - 1) ? tmp.back() : tmp[i + 1];
		inout[i]		  = (prevM + tmp[i] + nextM) / 3;
	}
}

class SPDSpectralMapperFactory : public ISpectralMapperFactory {
public:
	SPDSpectralMapperFactory(const SPDParameters& parameters)
		: mParameters(parameters)
		, mContext(mParameters.NumberOfBins)
		, mAlreadBuilt(false)
	{
	}

	std::shared_ptr<ISpectralMapper> createInstance(RenderContext* ctx) override
	{
		// TODO: We assume spectralStart, spectralEnd and ctx to never change. This is probably ok, but it should be better to check for special cases!
		mMutex.lock();
		if (!mAlreadBuilt) {
			buildDistribution(ctx);
			// TODO: Light distributions will not be used always... maybe make it optional??
			buildLightDistributions(ctx);
			mAlreadBuilt = true;
		}
		mMutex.unlock();

		if (mParameters.UseCMIS)
			return std::make_shared<CMISSPDSpectralMapper>(ctx->cameraSpectralRange(), ctx->lightSpectralRange(), mContext);
		else
			return std::make_shared<HeroSPDSpectralMapper>(ctx->cameraSpectralRange(), ctx->lightSpectralRange(), mContext);
	}

private:
	void calcLightDistribution(std::vector<float>& lightPower, const SpectralRange& range, Light* light)
	{
		const size_t bins  = lightPower.size();
		const auto bin2wvl = [=](uint32 bin) { return range.Start + (bin / float(bins - 1)) * range.span(); };

		// Calculate average power per spectral band
		for (uint32 i = 0; i < bins; i += PR_SPECTRAL_BLOB_SIZE) {
			const uint32 k = std::min<uint32>(bins - i, PR_SPECTRAL_BLOB_SIZE);

			SpectralBlob wavelengths = SpectralBlob::Zero();
			for (uint32 j = 0; j < k; ++j)
				wavelengths(j) = bin2wvl(i + j);
			// Let non active wavelengths at least have some reasonable garbage
			for (uint32 j = k; j < PR_SPECTRAL_BLOB_SIZE; ++j)
				wavelengths(j) = wavelengths(0);

			const SpectralBlob output = light->averagePower(wavelengths);

			for (uint32 j = 0; j < k; ++j)
				lightPower[i + j] = output(j);
		}

		// Normalize the given spd if necessary
		if (mParameters.UseNormalizedLights) {
			const float dt = 1.0f / (bins - 1);
			float integral = 0;
			for (float f : lightPower)
				integral += f * dt;
			if (integral > PR_EPSILON) {
				const float invnorm = 1 / integral;
				for (float& f : lightPower)
					f *= invnorm;
			}
		}
	}

	void applyPostprocessing(std::vector<float>& distr, const SpectralRange& range)
	{
		const size_t bins  = distr.size();
		const auto bin2wvl = [=](uint32 bin) { return range.Start + (bin / float(bins - 1)) * range.span(); };

		WeightingMethod method = mParameters.Method;
		// If the given spectral domain does not cross the cie domain, disable weighting
		if (range.Start > PR_CIE_WAVELENGTH_END || range.End < PR_CIE_WAVELENGTH_START)
			method = WeightingMethod::None;

		// Weight given spd entry by "camera" response
		switch (method) {
		case WeightingMethod::None:
			break;
		case WeightingMethod::CIE_Y:
			for (uint32 i = 0; i < bins; ++i)
				distr[i] *= CIE::eval_y(bin2wvl(i));
			break;
		case WeightingMethod::CIE_XYZ:
			for (uint32 i = 0; i < bins; ++i)
				distr[i] *= CIE::eval(bin2wvl(i)).sum();
			break;
		case WeightingMethod::SRGB:
			for (uint32 i = 0; i < bins; ++i) {
				const CIETriplet tripletXYZ = CIE::eval(bin2wvl(i));
				CIETriplet tripletRGB;
				RGBConverter::fromXYZ(tripletXYZ.data(), tripletRGB.data(), 3, 1);
				distr[i] *= tripletRGB.sum();
			}
			break;
		}

		// Smooth spd if necessary
		for (uint32 i = 0; i < mParameters.SmoothIterations; ++i)
			movingAverage(distr);

		// Ensure that all wavelengths will be sampled if necessary
		if (mParameters.EnsureCompleteSampling) {
			constexpr float MIN_POWER = 1e-3f;
			for (float& f : distr)
				f = std::max(MIN_POWER, f);
		}
	}

	void buildDistribution(RenderContext* ctx)
	{
		const SpectralRange cameraRange = ctx->cameraSpectralRange();
		const auto lightSampler			= ctx->lightSampler();

		// Temporary arrays
		std::vector<float> fullPower(mParameters.NumberOfBins, 0.0f);
		std::vector<float> lightPower(mParameters.NumberOfBins, 0.0f);

		for (const auto& light : lightSampler->lights()) {
			calcLightDistribution(lightPower, cameraRange, light.get());

			// Add it to the full spd
			for (uint32 i = 0; i < mParameters.NumberOfBins; ++i)
				fullPower[i] += lightPower[i];
		}

		applyPostprocessing(fullPower, cameraRange);

		if (mParameters.Verbose) {
			// Debug
			for (uint32 i = 0; i < mParameters.NumberOfBins - 1; ++i)
				std::cout << fullPower[i] << " ";
			std::cout << fullPower.back() << std::endl;
		}

		// Generate actual distribution
		mContext.Distribution.generate([&](uint32 bin) { return fullPower[bin]; });
	}

	void buildLightDistributions(RenderContext* ctx)
	{
		const SpectralRange cameraRange = ctx->cameraSpectralRange();
		const auto lightSampler			= ctx->lightSampler();

		if (lightSampler->lightCount() == 0)
			return;

		mContext.LightDistributions.resize(lightSampler->lightCount(), Distribution1D(mParameters.NumberOfLightBins));

		// Temporary arrays
		std::vector<float> lightPower(mParameters.NumberOfLightBins, 0.0f);

		for (const auto& light : lightSampler->lights()) {
			const SpectralRange range = light->spectralRange().bounded(cameraRange);

			calcLightDistribution(lightPower, range, light.get());
			applyPostprocessing(lightPower, range);

			mContext.LightDistributions[light->lightID()].generate([&](uint32 bin) { return lightPower[bin]; });
		}
	}
	const SPDParameters mParameters;

	SPDContext mContext;
	bool mAlreadBuilt;
	std::mutex mMutex;
};

class SPDSpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		SPDParameters parameters;
		parameters.NumberOfBins = ctx.parameters().getUInt("bins", parameters.NumberOfBins);

		std::string weighting = ctx.parameters().getString("weighting", "xyz");
		std::transform(weighting.begin(), weighting.end(), weighting.begin(), ::tolower);
		if (weighting == "none")
			parameters.Method = WeightingMethod::None;
		else if (weighting == "y")
			parameters.Method = WeightingMethod::CIE_Y;
		else if (weighting == "rgb" || weighting == "srgb")
			parameters.Method = WeightingMethod::SRGB;
		else
			parameters.Method = WeightingMethod::CIE_XYZ;

		parameters.EnsureCompleteSampling = ctx.parameters().getBool("complete", parameters.EnsureCompleteSampling);
		parameters.UseNormalizedLights	  = ctx.parameters().getBool("normalized", parameters.UseNormalizedLights);
		parameters.SmoothIterations		  = ctx.parameters().getUInt("smooth_iterations", parameters.SmoothIterations);
		parameters.UseCMIS				  = ctx.parameters().getBool("cmis", parameters.UseCMIS);
		parameters.Verbose				  = ctx.parameters().getBool("verbose", parameters.Verbose);

		return std::make_shared<SPDSpectralMapperFactory>(parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "spd", "default" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		const SPDParameters params;
		return PluginSpecificationBuilder("SPD Spectral Mapper", "Default, on spectral power distribution based spectral mapper")
			.Identifiers(getNames())
			.Inputs()
			.UInt("bins", "Number of bins in the histogram", params.NumberOfBins)
			.Option("weighting", "Weighting method", "xyz", { "none", "y", "rgb", "srgb", "xyz" })
			.Bool("complete", "Make sure all the wavelengths have a possibility to be sampled", params.EnsureCompleteSampling)
			.Bool("normalized", "Use normalized SPD for each light", params.UseNormalizedLights)
			.UInt("smooth_iterations", "Iteration count of smooth operation on accumulated histogram", params.SmoothIterations)
			.Bool("verbose", "Dump information regarding the accumulated SPD", params.Verbose)
			.Bool("cmis", "Use continuous MIS method. Should be on except for debug purposes", params.UseCMIS)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SPDSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)