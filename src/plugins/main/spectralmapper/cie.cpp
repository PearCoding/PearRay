#include "spectral/CIE.h"
#include "Environment.h"
#include "Random.h"
#include "SceneLoadContext.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"
#include "spectral/ISpectralMapperPlugin.h"

namespace PR {
template <bool YOnly>
class FullCIESpectralMapper : public ISpectralMapper {
public:
	FullCIESpectralMapper()
		: ISpectralMapper(PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END)
	{
	}

	virtual ~FullCIESpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		const float u = in.RND.getFloat();

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			if constexpr (YOnly)
				out.WavelengthNM(i) = CIE::sample_y(k, out.PDF(i));
			else
				out.WavelengthNM(i) = CIE::sample_xyz(k, out.PDF(i));
		}
	}

	SpectralBlob pdf(const SpectralEvalInput&, const SpectralBlob& wavelength) const override
	{
		SpectralBlob res;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			if constexpr (YOnly)
				res(i) = CIE::pdf_y(wavelength(i));
			else
				res(i) = CIE::pdf_xyz(wavelength(i));
		}
		return res;
	}
};

template <bool YOnly>
class TruncatedCIESpectralMapper : public ISpectralMapper {
public:
	TruncatedCIESpectralMapper(float spectralStart, float spectralEnd)
		: ISpectralMapper(spectralStart, spectralEnd)
	{
		PR_ASSERT(spectralStart >= PR_CIE_WAVELENGTH_START && spectralEnd <= PR_CIE_WAVELENGTH_END,
				  "Expected truncated domain to be inside cie domain");
	}

	virtual ~TruncatedCIESpectralMapper() = default;

	void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const override
	{
		const float u = in.RND.getFloat();

		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float k = std::fmod(u + i / (float)PR_SPECTRAL_BLOB_SIZE, 1.0f);
			if constexpr (YOnly)
				out.WavelengthNM(i) = CIE::sample_trunc_y(k, out.PDF(i), wavelengthStart(), wavelengthEnd());
			else
				out.WavelengthNM(i) = CIE::sample_trunc_xyz(k, out.PDF(i), wavelengthStart(), wavelengthEnd());
		}
	}

	SpectralBlob pdf(const SpectralEvalInput&, const SpectralBlob& wavelength) const override
	{
		SpectralBlob res;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			if constexpr (YOnly)
				res(i) = CIE::pdf_trunc_y(wavelength(i), wavelengthStart(), wavelengthEnd());
			else
				res(i) = CIE::pdf_trunc_xyz(wavelength(i), wavelengthStart(), wavelengthEnd());
		}
		return res;
	}
};

template <bool YOnly>
class CIESpectralMapperFactory : public ISpectralMapperFactory {
public:
	std::shared_ptr<ISpectralMapper> createInstance(float spectralStart, float spectralEnd, RenderContext*) override
	{
		if (spectralStart == PR_CIE_WAVELENGTH_START && spectralEnd == PR_CIE_WAVELENGTH_END)
			return std::make_shared<FullCIESpectralMapper<YOnly>>();
		else if (spectralStart >= PR_CIE_WAVELENGTH_START && spectralEnd <= PR_CIE_WAVELENGTH_END)
			return std::make_shared<TruncatedCIESpectralMapper<YOnly>>(spectralStart, spectralEnd);
		else
			return nullptr;
	}
};

class CIESpectralMapperPlugin : public ISpectralMapperPlugin {
public:
	std::shared_ptr<ISpectralMapperFactory> create(const std::string& type_name, const SceneLoadContext& ctx) override
	{
		if (type_name == "cie_y" || ctx.parameters().getBool("only_y", false))
			return std::make_shared<CIESpectralMapperFactory<true>>();
		else
			return std::make_shared<CIESpectralMapperFactory<false>>();
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "cie" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("CIE Spectral Mapper", "A spectral mapper based on the CIE curves. Only works for spectral ranges between the CIE specified range")
			.Identifiers(getNames())
			.Inputs()
			.Bool("only_y", "Use only the CIE Y curve", false)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CIESpectralMapperPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)