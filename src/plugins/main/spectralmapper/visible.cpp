#include "Environment.h"
#include "SceneLoadContext.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"

#include "Logger.h"
namespace PR {
template <bool YOnly>
class FullVisibleSpectralMapper : public ISpectralMapper {
public:
	FullVisibleSpectralMapper()
		: ISpectralMapper(PR_VISIBLE_WAVELENGTH_START, PR_VISIBLE_WAVELENGTH_END)
	{
	}

	virtual ~FullVisibleSpectralMapper() = default;

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			float pdf;
			if constexpr (YOnly)
				S.WavelengthNM(i) = CIE::sample_vis_y(k, pdf);
			else
				S.WavelengthNM(i) = CIE::sample_vis_xyz(k, pdf);
			S.PDF(i) = pdf;
		}

		return S;
	}

	SpectralBlob pdf(const Point2i&, const SpectralBlob& wavelength) const override
	{
		SpectralBlob res;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			if constexpr (YOnly)
				res(i) = CIE::pdf_vis_y(wavelength(i));
			else
				res(i) = CIE::pdf_vis_xyz(wavelength(i));
		}
		return res;
	}
};

template <bool YOnly>
class TruncatedVisibleSpectralMapper : public ISpectralMapper {
public:
	TruncatedVisibleSpectralMapper(float spectralStart, float spectralEnd)
		: ISpectralMapper(spectralStart, spectralEnd)
	{
		PR_ASSERT(spectralStart >= PR_VISIBLE_WAVELENGTH_START && spectralEnd <= PR_VISIBLE_WAVELENGTH_END,
				  "Expected truncated domain to be inside visible domain");
	}
	
	virtual ~TruncatedVisibleSpectralMapper() = default;

	SpectralMapSample sample(const Point2i&, float u) const override
	{
		SpectralMapSample S;

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			float pdf;
			if constexpr (YOnly)
				S.WavelengthNM(i) = CIE::sample_trunc_vis_y(k, pdf, wavelengthStart(), wavelengthEnd());
			else
				S.WavelengthNM(i) = CIE::sample_trunc_vis_xyz(k, pdf, wavelengthStart(), wavelengthEnd());
			S.PDF(i) = pdf;
		}

		return S;
	}

	SpectralBlob pdf(const Point2i&, const SpectralBlob& wavelength) const override
	{
		SpectralBlob res;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			if constexpr (YOnly)
				res(i) = CIE::pdf_trunc_vis_y(wavelength(i), wavelengthStart(), wavelengthEnd());
			else
				res(i) = CIE::pdf_trunc_vis_xyz(wavelength(i), wavelengthStart(), wavelengthEnd());
		}
		return res;
	}
};

template <bool YOnly>
class VisibleSpectralMapperFactory : public ISpectralMapperFactory {
public:
	std::shared_ptr<ISpectralMapper> createInstance(float spectralStart, float spectralEnd, RenderContext*) override
	{
		if (spectralStart == PR_VISIBLE_WAVELENGTH_START && spectralEnd == PR_VISIBLE_WAVELENGTH_END)
			return std::make_shared<FullVisibleSpectralMapper<YOnly>>();
		else if (spectralStart >= PR_VISIBLE_WAVELENGTH_START && spectralEnd <= PR_VISIBLE_WAVELENGTH_END) {
			PR_LOG(L_INFO) << "Truncated spectral mapper" << std::endl;
			return std::make_shared<TruncatedVisibleSpectralMapper<YOnly>>(spectralStart, spectralEnd);
		} else
			return nullptr;
	}
};

class VisibleSpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(const std::string& name, const SceneLoadContext& ctx) override
	{
		if (name == "visible_y" || ctx.parameters().getBool("only_y", false))
			return std::make_shared<VisibleSpectralMapperFactory<true>>();
		else
			return std::make_shared<VisibleSpectralMapperFactory<false>>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "visible", "visible_y", "visible_xyz"/*, "visible_rgb"*/ });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::VisibleSpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)