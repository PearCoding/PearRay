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

// Spectral mapper based on the function described in paper
// An Improved Technique for Full Spectral Rendering
namespace PR {
constexpr float AStd = 0.0072f;
constexpr float BStd = 538;

static inline float aghSingle(float lambda, float A, float B)
{
	return std::tanh(A * (lambda - B));
}

static inline float aghPDF(float lambda, float A, float B, float N, float C)
{
	return (aghSingle(lambda, A, B) - C) / N;
}

static inline float aghSample(float u, float A, float B, float N, float C)
{
	return B - std::atanh(N * u + C) / A;
}

class CMISAGHSpectralMapper : public ISpectralMapper {
public:
	CMISAGHSpectralMapper(const SpectralRange& cameraRange, const SpectralRange& lightRange)
		: ISpectralMapper()
		, mCameraRange(cameraRange)
		, mLightRange(lightRange)
		, mCameraC(aghSingle(cameraRange.Start, AStd, BStd))
		, mCameraN(aghSingle(cameraRange.End, AStd, BStd) - aghSingle(cameraRange.Start, AStd, BStd)) // A already included
	{
	}

	virtual ~CMISAGHSpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			const float u = in.RND.getFloat();
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
				const float k		= std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
				out.WavelengthNM(i) = aghSample(k, AStd, BStd, mCameraN, mCameraC);
			}
		} else {
			out.WavelengthNM = sampleStandardWavelength(in.RND.getFloat(), mLightRange, out.PDF);
		}
	}

	// Do not apply the jacobian of the mapping from [0, 1] to [start, end], as we do not divide by it
	SpectralBlob pdf(const SpectralEvalInput& in, const SpectralBlob& wavelength) const override
	{
		if (in.Purpose == SpectralSamplePurpose::Pixel) {
			SpectralBlob res;
			PR_OPT_LOOP
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				res(i) = aghPDF(wavelength(i), AStd, BStd, mCameraN, mCameraC);
			return res;
		} else {
			// For connections and cases with not enough information, we fallback to pure random sampling
			return pdfStandardWavelength(wavelength, mLightRange);
		}
	}

private:
	const SpectralRange mCameraRange;
	const SpectralRange mLightRange;
	const float mCameraC;
	const float mCameraN;
};

// TODO: Hero

struct AGHParameters {
};

class AGHSpectralMapperFactory : public ISpectralMapperFactory {
public:
	AGHSpectralMapperFactory(const AGHParameters& parameters)
		: mParameters(parameters)
	{
	}

	std::shared_ptr<ISpectralMapper> createInstance(RenderContext* ctx) override
	{
		return std::make_shared<CMISAGHSpectralMapper>(ctx->cameraSpectralRange(), ctx->lightSpectralRange());
	}

private:
	const AGHParameters mParameters;
};

class AGHSpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		AGHParameters parameters;

		return std::make_shared<AGHSpectralMapperFactory>(parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "agh" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		const AGHParameters params;
		return PluginSpecificationBuilder("AGH Spectral Mapper", "Spectral mapper based on the paper 'An Improved Technique for Full Spectral Rendering'")
			.Identifiers(getNames())
			.Inputs()
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::AGHSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)