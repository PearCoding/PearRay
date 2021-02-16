#include "Random.h"
#include "Environment.h"
#include "SceneLoadContext.h"
#include "renderer/RenderContext.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"

namespace PR {
class RandomSpectralMapper : public ISpectralMapper {
public:
	RandomSpectralMapper(const SpectralRange& cameraRange, const SpectralRange& lightRange)
		: ISpectralMapper()
		, mCameraRange(cameraRange)
		, mLightRange(lightRange)
	{
	}

	virtual ~RandomSpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		const SpectralRange& range = in.Purpose == SpectralSamplePurpose::Pixel ? mCameraRange : mLightRange;
		const float u			   = in.RND.getFloat();

		const float span  = range.span();
		const float delta = span / PR_SPECTRAL_BLOB_SIZE;

		const float start	= u * span;			   // Wavelength inside the span
		out.WavelengthNM(0) = start + range.Start; // Hero wavelength
		PR_OPT_LOOP
		for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			out.WavelengthNM(i) = range.Start + std::fmod(start + i * delta, span);
		out.PDF = 1;
	}

private:
	SpectralRange mCameraRange;
	SpectralRange mLightRange;
};

class RandomSpectralMapperFactory : public ISpectralMapperFactory {
public:
	std::shared_ptr<ISpectralMapper> createInstance(RenderContext* ctx) override
	{
		return std::make_shared<RandomSpectralMapper>(ctx->cameraSpectralRange(), ctx->lightSpectralRange());
	}
};

class RandomSpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(const std::string&, const SceneLoadContext&) override
	{
		return std::make_shared<RandomSpectralMapperFactory>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "random" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Random Spectral Mapper", "A pure random spectral mapper")
			.Identifiers(getNames())
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RandomSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)