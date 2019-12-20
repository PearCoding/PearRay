#include "image_io.h"
#include "Logger.h"

#include "spectral/RGBConverter.h"
#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include <OpenImageIO/imageio.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

OIIO_NAMESPACE_USING
namespace bf = boost::filesystem;

namespace PR {
void save_normalized_gray(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data)
{
	float max = 0;
	for (size_t i = 0; i < data.size(); ++i)
		max = std::max(max, data[i]);

	if (max <= 1.0f) {
		save_gray(path, xres, yres, data);
	} else {
		std::vector<float> copy = data;
		for (size_t i = 0; i < data.size(); ++i)
			copy[i] /= max;

		save_gray(path, xres, yres, copy);
	}
}

void save_gray(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data)
{
	save_image(path, xres, yres, data, 1);
}

void save_image(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data, size_t channels)
{
#if OIIO_PLUGIN_VERSION >= 22
	std::unique_ptr<ImageOutput> out = ImageOutput::create(path);
#else
	ImageOutput* out = ImageOutput::create(path);
#endif
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec spec((int)xres, (int)yres, (int)channels, TypeDesc::FLOAT);
	out->open(path, spec);
	out->write_image(TypeDesc::FLOAT, data.data());
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	ImageOutput::destroy(out);
#endif
}

void save_spec(const std::string& path,
			   const Spectrum& spec)
{
	constexpr float MAX_INTENSITY = 2.0f;
	const size_t xres = spec.samples(), yres = 100;
	const size_t channels = 3;
	const size_t size	 = xres * yres * channels;
	std::vector<uint8> pixels(size, 255);

	for (uint32 i = 0; i < xres; ++i) {
		const float f = spec(i);

		if (f < 0) {
			pixels[(yres - 1) * xres * 3 + i * 3 + 0] = 255;
			pixels[(yres - 1) * xres * 3 + i * 3 + 1] = 0;
			pixels[(yres - 1) * xres * 3 + i * 3 + 2] = 0;
		} else if (f > MAX_INTENSITY) {
			pixels[0 * xres * 3 + i * 3 + 0] = 0;
			pixels[0 * xres * 3 + i * 3 + 1] = 255;
			pixels[0 * xres * 3 + i * 3 + 2] = 0;
		} else {
			const uint32 y = yres - std::min<uint32>(yres - 1, std::max<uint32>(0, static_cast<uint32>(yres * f / MAX_INTENSITY))) - 1;

			pixels[y * xres * 3 + i * 3 + 0] = 0;
			pixels[y * xres * 3 + i * 3 + 1] = 0;
			pixels[y * xres * 3 + i * 3 + 2] = 255;
		}
	}

#if OIIO_PLUGIN_VERSION >= 22
	std::unique_ptr<ImageOutput> out = ImageOutput::create(path);
#else
	ImageOutput* out = ImageOutput::create(path);
#endif
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec imgSpec((int)xres, (int)yres, (int)channels, TypeDesc::UINT8);
	out->open(path, imgSpec);
	out->write_image(TypeDesc::UINT8, pixels.data());
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	ImageOutput::destroy(out);
#endif
}

void save_color_preview(const std::string& path,
						float r, float g, float b,
						float r2, float g2, float b2)
{
	const size_t xres = 32, yres = 32;
	const size_t channels = 3;
	uint8 pixels[xres * yres * channels];

	for (uint32 i = 0; i < xres / 2; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			pixels[j * xres * 3 + i * 3 + 0] = int(r * 255);
			pixels[j * xres * 3 + i * 3 + 1] = int(g * 255);
			pixels[j * xres * 3 + i * 3 + 2] = int(b * 255);
		}
	}

	for (uint32 i = xres / 2; i < xres; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			pixels[j * xres * 3 + i * 3 + 0] = int(r2 * 255);
			pixels[j * xres * 3 + i * 3 + 1] = int(g2 * 255);
			pixels[j * xres * 3 + i * 3 + 2] = int(b2 * 255);
		}
	}

#if OIIO_PLUGIN_VERSION >= 22
	std::unique_ptr<ImageOutput> out = ImageOutput::create(path);
#else
	ImageOutput* out = ImageOutput::create(path);
#endif
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec spec((int)xres, (int)yres, (int)channels, TypeDesc::UINT8);
	out->open(path, spec);
	out->write_image(TypeDesc::UINT8, pixels);
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	ImageOutput::destroy(out);
#endif
}
} // namespace PR