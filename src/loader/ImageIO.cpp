#include "ImageIO.h"
#include "config/Build.h"

#include <OpenImageIO/imageio.h>

namespace PR {
bool ImageIO::save(const std::filesystem::path& path, const float* data, size_t width, size_t height, size_t channels, const ImageSaveOptions& opts)
{
	OIIO::ImageSpec spec(width, height, channels, OIIO::TypeDesc::FLOAT);

	const std::string versionStr = Build::getVersionString();
	spec.attribute("PixelAspectRatio", 1.0f);
	spec.attribute("Software", "PearRay " + versionStr);
	spec.attribute("IPTC:ProgramVersion", versionStr);
	if (opts.Parametric) {
		spec.attribute("oiio:ColorSpace", "PRParametric");
		spec.attribute("oiio:Gamma", 1.0f);
		spec.attribute("PR:Parametric", "true");
		spec.attribute("oiio:RawColor", 1);
		spec.attribute("compression", "piz");

		spec.channelnames.clear();
		spec.channelnames.push_back("coeff.A");
		spec.channelnames.push_back("coeff.B");
		spec.channelnames.push_back("coeff.C");
	} else {
		spec.attribute("compression", "pxr24");
	}

	if (channels > 3) {
		spec.channelnames.resize(channels);
		for (size_t i = 0; i < channels; ++i)
			spec.channelnames[i] = "Value_" + std::to_string(i + 1);
	}

	auto out = OIIO::ImageOutput::create(path.string());
	if (!out)
		return false;

	if (!out->open(path, spec)) {
#if OIIO_PLUGIN_VERSION < 22
		OIIO::ImageOutput::destroy(out);
#endif
		return false;
	}

	out->write_image(OIIO::TypeDesc::FLOAT, data);
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	OIIO::ImageOutput::destroy(out);
#endif

	return true;
}

bool ImageIO::load(const std::filesystem::path& path, std::vector<float>& data, size_t& width, size_t& height, size_t& channels)
{
	auto in = OIIO::ImageInput::open(path.string());
	if (!in)
		return false;

	const OIIO::ImageSpec spec = in->spec();

	width	 = std::max(0, spec.width);
	height	 = std::max(0, spec.height);
	channels = std::max(0, spec.nchannels);

	if (width * height * channels == 0) {
#if OIIO_PLUGIN_VERSION < 22
		OIIO::ImageInput::destroy(in);
#endif
		return false;
	}

	const float gamma = spec.get_float_attribute("oiio:Gamma", 1.0f);
	const bool isSRGB = spec.get_string_attribute("oiio:ColorSpace") == "sRGB";

	data.resize(width * height * channels);

	in->read_image(OIIO::TypeDesc::FLOAT, data.data());
	in->close();

#if OIIO_PLUGIN_VERSION < 22
	OIIO::ImageInput::destroy(in);
#endif

	// Map to linear
	if (isSRGB) {
		const auto map = [](float u) {
			if (u <= 0.04045f)
				return u / 12.92f;
			else
				return std::pow((u + 0.055f) / 1.055f, 2.4f);
		};
		PR_OPT_LOOP
		for (float& v : data)
			v = map(v);
	} else if (gamma != 1.0f) {
		const float invGamma = 1 / gamma;
		PR_OPT_LOOP
		for (float& v : data)
			v = std::pow(v, invGamma);
	}

	return true;
}
} // namespace PR