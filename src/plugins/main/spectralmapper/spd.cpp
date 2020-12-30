#include "Environment.h"
#include "SceneLoadContext.h"
#include "light/LightSampler.h"
#include "renderer/RenderContext.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"
#include "spectral/RGBConverter.h"

#include "Logger.h"

#include <mutex>

namespace PR {
class SPDSpectralMapper : public ISpectralMapper {
public:
	SPDSpectralMapper(float start, float end, const Distribution1D& distribution)
		: ISpectralMapper(start, end)
		, mDistribution(distribution)
	{
	}

	virtual ~SPDSpectralMapper() = default;

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			float pdf;
			S.WavelengthNM(i) = mDistribution.sampleContinuous(u, pdf) * (wavelengthEnd() - wavelengthStart()) + wavelengthStart();
			S.PDF(i)		  = pdf;
		}

		return S;
	}

	// Do not apply the jacobian of the mapping from [0, 1] to [start, end], as we do not divide by it
	SpectralBlob pdf(const Point2i&, const SpectralBlob& wavelength) const override
	{
		SpectralBlob res;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res(i) = mDistribution.continuousPdf((wavelength(i) - wavelengthStart()) / (wavelengthEnd() - wavelengthStart()));
		return res;
	}

private:
	const Distribution1D mDistribution;
};

enum WeightingMethod {
	WM_None = 0,
	WM_CIE_Y,
	WM_CIE_XYZ,
	WM_SRGB,
};

struct SPDParameters {
	uint32 NumberOfBins			= 256 /* This one can be high quality*/;
	WeightingMethod Method		= WM_CIE_XYZ;
	bool UseNormalizedLights	= true;
	bool EnsureCompleteSampling = true;
	uint32 SmoothIterations		= 0; // How many times to apply moving average?
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
		, mDistribution(mParameters.NumberOfBins)
		, mAlreadBuilt(false)
	{
	}

	std::shared_ptr<ISpectralMapper> createInstance(float spectralStart, float spectralEnd, RenderContext* ctx) const override
	{
		WeightingMethod method = mParameters.Method;
		// If the given spectral domain does not cross the cie domain, disable weighting
		if (spectralStart > PR_CIE_WAVELENGTH_END || spectralEnd < PR_CIE_WAVELENGTH_START)
			method = WM_None;

		const auto bin2wvl = [=](uint32 bin) { return spectralStart + (bin / float(mParameters.NumberOfBins - 1)) * (spectralEnd - spectralStart); };

		// TODO: We assume spectralStart, spectralEnd and ctx to never change. This is probably ok, but it should be better to check for special cases!
		mMutex.lock();
		if (!mAlreadBuilt) {
			const auto lightSampler = ctx->lightSampler();

			// Temporary arrays
			std::vector<float> fullPower(mParameters.NumberOfBins, 0.0f);
			std::vector<float> lightPower(mParameters.NumberOfBins, 0.0f);

			for (const auto& light : lightSampler->lights()) {
				// Calculate average power per spectral band
				for (uint32 i = 0; i < mParameters.NumberOfBins; i += PR_SPECTRAL_BLOB_SIZE) {
					const uint32 k = std::min<uint32>(mParameters.NumberOfBins - i, PR_SPECTRAL_BLOB_SIZE);

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
					const float dt = 1.0f / (mParameters.NumberOfBins - 1);
					float integral = 0;
					for (float f : lightPower)
						integral += f * dt;
					if (integral > PR_EPSILON) {
						const float invnorm = 1 / integral;
						for (float& f : lightPower)
							f *= invnorm;
					}
				}

				// Add it to the full spd
				for (uint32 i = 0; i < mParameters.NumberOfBins; ++i)
					fullPower[i] += lightPower[i];
			}

			// Weight given spd entry by "camera" response
			switch (method) {
			case WM_None:
				break;
			case WM_CIE_Y:
				for (uint32 i = 0; i < mParameters.NumberOfBins; ++i)
					fullPower[i] *= CIE::eval_y(bin2wvl(i));
				break;
			case WM_CIE_XYZ:
				for (uint32 i = 0; i < mParameters.NumberOfBins; ++i)
					fullPower[i] *= CIE::eval(bin2wvl(i)).sum();
				break;
			case WM_SRGB:
				for (uint32 i = 0; i < mParameters.NumberOfBins; ++i) {
					const CIETriplet tripletXYZ = CIE::eval(bin2wvl(i));
					CIETriplet tripletRGB;
					RGBConverter::fromXYZ(tripletXYZ.data(), tripletRGB.data(), 3, 1);
					fullPower[i] *= tripletRGB.sum();
				}
				break;
			}

			// Smooth spd if necessary
			for (uint32 i = 0; i < mParameters.SmoothIterations; ++i)
				movingAverage(fullPower);

			// Ensure that all wavelengths will be sampled if necessary
			if (mParameters.EnsureCompleteSampling) {
				constexpr float MIN_POWER = 1e-3f;
				for (float& f : fullPower)
					f = std::max(MIN_POWER, f);
			}

			// Debug
			for (uint32 i = 0; i < mParameters.NumberOfBins - 1; ++i)
				std::cout << fullPower[i] << " ";
			std::cout << fullPower.back() << std::endl;

			// Generate actual distribution
			mDistribution.generate([&](uint32 bin) { return fullPower[bin]; });

			mAlreadBuilt = true;
		}
		mMutex.unlock();

		return std::make_shared<SPDSpectralMapper>(spectralStart, spectralEnd, mDistribution);
	}

private:
	const SPDParameters mParameters;

	mutable Distribution1D mDistribution;
	mutable bool mAlreadBuilt;
	mutable std::mutex mMutex;
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
			parameters.Method = WM_None;
		else if (weighting == "y")
			parameters.Method = WM_CIE_Y;
		else if (weighting == "rgb" || weighting == "srgb")
			parameters.Method = WM_SRGB;
		else
			parameters.Method = WM_CIE_XYZ;

		parameters.EnsureCompleteSampling = ctx.parameters().getBool("complete", parameters.EnsureCompleteSampling);
		parameters.UseNormalizedLights	  = ctx.parameters().getBool("normalized", parameters.UseNormalizedLights);
		parameters.SmoothIterations		  = ctx.parameters().getUInt("smooth_iterations", parameters.SmoothIterations);

		return std::make_shared<SPDSpectralMapperFactory>(parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "spd", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SPDSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)