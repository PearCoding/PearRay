#include "spectral/CIE.h"
#include "Environment.h"
#include "SceneLoadContext.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"

namespace PR {
class FullCIESpectralMapper : public ISpectralMapper {
public:
	FullCIESpectralMapper()
		: ISpectralMapper(PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END)
	{
	}

	virtual ~FullCIESpectralMapper() = default;

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			float pdf;
#ifdef PR_SAMPLE_BY_CIE_Y
			S.WavelengthNM(i) = CIE::sample_y(k, pdf);
#else
			S.WavelengthNM(i) = CIE::sample_xyz(k, pdf);
#endif
			S.PDF(i) = pdf;
		}

		return S;
	}
};

class TruncatedCIESpectralMapper : public ISpectralMapper {
public:
	TruncatedCIESpectralMapper(float spectralStart, float spectralEnd)
		: ISpectralMapper(spectralStart, spectralEnd)
	{
		PR_ASSERT(spectralStart >= PR_CIE_WAVELENGTH_START && spectralEnd <= PR_CIE_WAVELENGTH_END,
				  "Expected truncated domain to be inside cie domain");
	}

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			float pdf;
#ifdef PR_SAMPLE_BY_CIE_Y
			S.WavelengthNM(i) = CIE::sample_trunc_y(k, pdf, wavelengthStart(), wavelengthEnd());
#else
			S.WavelengthNM(i) = CIE::sample_trunc_xyz(k, pdf, wavelengthStart(), wavelengthEnd());
#endif
			S.PDF(i) = pdf;
		}

		return S;
	}
};

class CIESpectralMapperFactory : public ISpectralMapperFactory {
public:
	std::shared_ptr<ISpectralMapper> createInstance(float spectralStart, float spectralEnd, RenderContext*) const override
	{
		if (spectralStart == PR_CIE_WAVELENGTH_START && spectralEnd == PR_CIE_WAVELENGTH_END)
			return std::make_shared<FullCIESpectralMapper>();
		else if (spectralStart >= PR_CIE_WAVELENGTH_START && spectralEnd <= PR_CIE_WAVELENGTH_END)
			return std::make_shared<TruncatedCIESpectralMapper>(spectralStart, spectralEnd);
		else
			return nullptr;
	}
};

class CIESpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(uint32, const std::string&, const SceneLoadContext&) override
	{
		return std::make_shared<CIESpectralMapperFactory>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "cie" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CIESpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)