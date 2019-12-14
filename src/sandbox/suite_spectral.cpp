#include "Logger.h"
#include "image_io.h"

#include "spectral/RGBConverter.h"
#include "spectral/SpectrumDescriptor.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

using namespace PR;
namespace bf = boost::filesystem;

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
	save_color_preview(stream.str() + ".png", r, g, b, R, G, B);
	save_spec(stream.str() + "_spec.png", spec);
}

void suite_spectral()
{
	bf::create_directory("results/spectral");

	std::ofstream log("results/spectral/errors.log");

	maxE = 0;
	minE = std::numeric_limits<float>::infinity();

	constexpr size_t count = 11;
	constexpr float step   = 1.0f / (count - 1);
	for (size_t r = 0; r < count; ++r) {
		for (size_t g = 0; g < count; ++g) {
			for (size_t b = 0; b < count; ++b) {
				handle_color(r * step, g * step, b * step, log);
			}
		}
	}

	log << std::endl
		<< "MAX = " << maxE << std::endl
		<< "MIN = " << minE << std::endl;
}