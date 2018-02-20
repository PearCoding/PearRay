#include "Logger.h"

#include "spectral/RGBConverter.h"
#include "spectral/SpectrumDescriptor.h"

#include <OpenImageIO/imageio.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

OIIO_NAMESPACE_USING
using namespace PR;
namespace bf = boost::filesystem;

static void save_spec(const std::string& path,
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
			pixels[(yres - 1) * xres * 3 + i * 3]	 = 255;
			pixels[(yres - 1) * xres * 3 + i * 3 + 1] = 0;
			pixels[(yres - 1) * xres * 3 + i * 3 + 2] = 0;
		} else if (f > MAX_INTENSITY) {
			pixels[0 * xres * 3 + i * 3]	  = 0;
			pixels[0 * xres * 3 + i * 3 + 1] = 255;
			pixels[0 * xres * 3 + i * 3 + 2]  = 0;
		} else {
			uint32 y = yres - std::min<uint32>(yres - 1, std::max<uint32>(0, yres * f / MAX_INTENSITY)) - 1;

			pixels[y * xres * 3 + i * 3]	 = 0;
			pixels[y * xres * 3 + i * 3 + 1] = 0;
			pixels[y * xres * 3 + i * 3 + 2] = 255;
		}
	}

	ImageOutput* out = ImageOutput::create(path);
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec imgSpec(xres, yres, channels, TypeDesc::UINT8);
	out->open(path, imgSpec);
	out->write_image(TypeDesc::UINT8, pixels.data());
	out->close();

	ImageOutput::destroy(out);
}

static void save_image(const std::string& path,
					   float r, float g, float b,
					   float r2, float g2, float b2)
{
	const size_t xres = 32, yres = 32;
	const size_t channels = 3;
	uint8 pixels[xres * yres * channels];

	for (uint32 i = 0; i < xres / 2; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			pixels[j * xres * 3 + i * 3]	 = int(r * 255);
			pixels[j * xres * 3 + i * 3 + 1] = int(g * 255);
			pixels[j * xres * 3 + i * 3 + 2] = int(b * 255);
		}
	}

	for (uint32 i = xres / 2; i < xres; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			pixels[j * xres * 3 + i * 3]	 = int(r2 * 255);
			pixels[j * xres * 3 + i * 3 + 1] = int(g2 * 255);
			pixels[j * xres * 3 + i * 3 + 2] = int(b2 * 255);
		}
	}

	ImageOutput* out = ImageOutput::create(path);
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);
	out->open(path, spec);
	out->write_image(TypeDesc::UINT8, pixels);
	out->close();

	ImageOutput::destroy(out);
}

static float maxE = 0;
static float minE = std::numeric_limits<float>::infinity();

static void handle_color(float r, float g, float b, std::ofstream& log)
{
	std::shared_ptr<SpectrumDescriptor> desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec(desc);

	PR::RGBConverter::toSpec(spec, r, g, b);
	float R, G, B;
	PR::RGBConverter::convert(spec, R, G, B);

	float err = std::sqrt(std::pow(r - R, 2) + std::pow(g - G, 2) + std::pow(b - B, 2));
	log << "[" << r << "," << g << "," << b << "]=" << err << std::endl;
	maxE = std::max(err, maxE);
	minE = std::min(err, minE);

	std::stringstream stream;
	stream << "results/spectral1/"
		   << int(r * 255) << "_" << int(g * 255) << "_" << int(b * 255);
	save_image(stream.str() + ".png", r, g, b, R, G, B);
	save_spec(stream.str() + "_spec.png", spec);
}

void suite_spectral1()
{
	bf::create_directory("results/spectral1");

	std::ofstream log("results/spectral1/errors.log");

	maxE = 0;
	minE = std::numeric_limits<float>::infinity();

    constexpr size_t count = 11;
	constexpr float step = 1.0f/(count-1);
	for (size_t r = 0; r < count; ++r) {
		for (size_t g = 0; g < count; ++g) {
			for (size_t b = 0; b < count; ++b) {
				handle_color(r*step, g*step, b*step, log);
			}
		}
	}

	log << std::endl
		<< "MAX = " << maxE << std::endl
		<< "MIN = " << minE << std::endl;
}