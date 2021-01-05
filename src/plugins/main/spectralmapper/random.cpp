#include "Environment.h"
#include "SceneLoadContext.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"

namespace PR {
class RandomSpectralMapper : public ISpectralMapper {
public:
	RandomSpectralMapper(float spectralStart, float spectralEnd)
		: ISpectralMapper(spectralStart, spectralEnd)
	{
	}

	virtual ~RandomSpectralMapper() = default;

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		const float span  = wavelengthEnd() - wavelengthStart();
		const float delta = span / PR_SPECTRAL_BLOB_SIZE;

		const float start = u * span;				   // Wavelength inside the span
		S.WavelengthNM(0) = start + wavelengthStart(); // Hero wavelength
		PR_OPT_LOOP
		for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			S.WavelengthNM(i) = wavelengthStart() + std::fmod(start + i * delta, span);
		S.PDF = 1;

		return S;
	}
};

class RandomSpectralMapperFactory : public ISpectralMapperFactory {
public:
	std::shared_ptr<ISpectralMapper> createInstance(float spectralStart, float spectralEnd, RenderContext*) override
	{
		return std::make_shared<RandomSpectralMapper>(spectralStart, spectralEnd);
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

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::RandomSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)