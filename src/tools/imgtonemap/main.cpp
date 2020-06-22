#include <filesystem>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <OpenImageIO/imageio.h>
#include <cxxopts.hpp>

#define USE_MEDIAN_FOR_LUMINANCE_ESTIMATION

namespace sf = std::filesystem;

class ProgramSettings {
public:
	sf::path InputFile;
	sf::path OutputFile;

	bool IsVerbose;
	bool IsQuiet;
	bool InputIsLinear;
	bool OutputIsLinear;
	bool ClampOutput;

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("prtonemap", "Apply tonemapping to an image");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("q,quiet", "Do not print messages into console")
			("v,verbose", "Print detailed information")
			("i,input", "Input file", cxxopts::value<std::string>())
			("o,output", "Output file", cxxopts::value<std::string>())
			("correct-input", "Gamma correct the input image")
			("linear", "Output will be in linear sRGB space")
			("clamp", "Clamp output components inside [0,1]")
		;
		// clang-format on
		options.parse_positional({ "input", "output" });

		auto vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
					  << std::endl;
			std::cout << options.help() << std::endl;
			exit(0);
		}

		// Handle version
		if (!vm.count("input")) {
			std::cout << "No input given!" << std::endl;
			return false;
		}

		// Input file 1
		InputFile = vm["input"].as<std::string>();
		if (!sf::exists(InputFile)) {
			std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
			return false;
		}

		// Output file
		if (!vm.count("output")) {
			std::cout << "No output given!" << std::endl;
			return false;
		}
		OutputFile = vm["output"].as<std::string>();

		IsVerbose	   = (vm.count("verbose") != 0);
		IsQuiet		   = (vm.count("quiet") != 0);
		InputIsLinear  = (vm.count("correct-input") == 0);
		OutputIsLinear = (vm.count("linear") != 0);
		ClampOutput	   = (vm.count("clamp") != 0);
	} catch (const cxxopts::OptionException& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	return true;
}

struct ImageInfo {
	size_t RStride;
	size_t GStride;
	size_t BStride;
	size_t Width;
	size_t Height;
	size_t Channels;
};

bool read_input(const std::string& filename, std::vector<float>& data, ImageInfo& info)
{
	auto in = OIIO::ImageInput::open(filename);
	if (!in) {
		std::cerr << "Error: Could not open input file" << std::endl;
		return false;
	}
	const OIIO::ImageSpec& spec = in->spec();
	info.Width					= spec.width;
	info.Height					= spec.height;
	info.Channels				= spec.nchannels;

	bool hasR = false;
	bool hasG = false;
	bool hasB = false;

	for (int ch = 0; ch < spec.nchannels; ++ch) {
		const auto name = spec.channel_name(ch);
		if (name == "R") {
			hasR		 = true;
			info.RStride = ch;
		} else if (name == "G") {
			hasG		 = true;
			info.GStride = ch;
		} else if (name == "B") {
			hasB		 = true;
			info.BStride = ch;
		}
	}

	if (!hasR || !hasG || !hasB) {
		std::cerr << "Error: Could not find all RGB channels" << std::endl;
		return false;
	}

	data.resize(info.Width * info.Height * info.Channels);
	in->read_image(0, spec.nchannels, OIIO::TypeDesc::FLOAT, &data[0]);
	in->close();

	return true;
}

bool write_output(const std::string& filename, const std::vector<float>& data, int width, int height)
{
	auto out = OIIO::ImageOutput::create(filename);
	if (!out) {
		std::cerr << "Error: Could not create output file" << std::endl;
		return false;
	}

	OIIO::ImageSpec spec(width, height, 3, OIIO::TypeDesc::FLOAT);
	spec.channelnames.clear();
	spec.channelnames.push_back("R");
	spec.channelnames.push_back("G");
	spec.channelnames.push_back("B");
	spec.attribute("oiio:ColorSpace", "Linear");
	spec.attribute("Software", "PearRay imgdiff tool");

	out->open(filename, spec);
	out->write_image(OIIO::TypeDesc::FLOAT, &data[0]);
	out->close();

	return true;
}

static inline float srgb_from_linear(float u)
{
	return u <= 0.0031308f ? 12.92f * u : (1.055f * std::pow(u, 1 / 2.4f) - 0.055f);
}

static inline float srgb_to_linear(float u)
{
	return u <= 0.004045f ? u / 12.92f : (std::pow(u + 0.055f, 2.4f) / 1.055f);
}

static inline void xyz_to_srgb(float x, float y, float z, float& r, float& g, float& b)
{
	r = 3.2404542f * x - 1.5371385f * y - 0.4985314f * z;
	g = -0.9692660f * x + 1.8760108f * y + 0.0415560f * z;
	b = 0.0556434f * x - 0.2040259f * y + 1.0572252f * z;
}

static inline void srgb_to_xyz(float r, float g, float b, float& x, float& y, float& z)
{
	x = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
	y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
	z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;
}

static inline void xyY_to_srgb(float x, float y, float Y, float& r, float& g, float& b)
{
	if (y == 0) {
		r = 0;
		g = 0;
		b = 0;
	} else {
		xyz_to_srgb(x * Y / y, Y, (1 - x - y) * Y / y, r, g, b);
	}
}

static inline void srgb_to_xyY(float r, float g, float b, float& x, float& y, float& Y)
{
	float z;
	srgb_to_xyz(r, g, b, x, y, z);
	const auto n = x + y + z;
	if (n != 0) {
		Y = y;
		x /= n;
		y /= n;
	} else {
		Y = 0;
		x = 0;
		y = 0;
	}
}

[[maybe_unused]] static inline float reinhard(float L)
{
	return L / (1.0f + L);
}

[[maybe_unused]] static inline float reinhard_modified(float L)
{
	constexpr float WhitePoint = 4.0f;
	return (L * (1.0f + L / (WhitePoint * WhitePoint))) / (1.0f + L);
}

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	if (!options.IsQuiet && options.IsVerbose) {
		std::cout << "Arguments> " << std::endl;
		std::cout << "  Input:  " << options.InputFile << std::endl;
		std::cout << "  Output: " << options.OutputFile << std::endl;
	}

	std::vector<float> in_data;
	ImageInfo info;
	if (!read_input(options.InputFile.generic_string(), in_data, info))
		return EXIT_FAILURE;

	if (!options.InputIsLinear) {
		for (size_t i = 0; i < info.Width * info.Height; ++i) {
			float& R = in_data[i * info.Channels + info.RStride];
			float& G = in_data[i * info.Channels + info.GStride];
			float& B = in_data[i * info.Channels + info.BStride];

			R = srgb_to_linear(R);
			G = srgb_to_linear(G);
			B = srgb_to_linear(B);
		}
	}

	float max_luminance = 0;
#ifdef USE_MEDIAN_FOR_LUMINANCE_ESTIMATION
	constexpr size_t WINDOW_S = 3;
	constexpr size_t EDGE_S	  = WINDOW_S / 2;
	std::array<float, WINDOW_S * WINDOW_S> window;

	for (size_t y = EDGE_S; y < info.Height - EDGE_S; ++y) {
		for (size_t x = EDGE_S; x < info.Width - EDGE_S; ++x) {

			size_t i = 0;
			for (size_t wy = 0; wy < WINDOW_S; ++wy) {
				for (size_t wx = 0; wx < WINDOW_S; ++wx) {
					const auto ix = x + wx - EDGE_S;
					const auto iy = y + wy - EDGE_S;

					auto r = in_data[(iy * info.Width + ix) * 3 + 0];
					auto g = in_data[(iy * info.Width + ix) * 3 + 1];
					auto b = in_data[(iy * info.Width + ix) * 3 + 2];

					float x, y, Y;
					srgb_to_xyY(r, g, b, x, y, Y);
					window[i] = Y;
					++i;
				}
			}

			std::sort(window.begin(), window.end());
			const auto L  = window[window.size() / 2];
			max_luminance = std::max(max_luminance, L);
		}
	}
#else
	for (size_t i = 0; i < info.Width * info.Height; ++i) {
		const float R = in_data[i * info.Channels + info.RStride];
		const float G = in_data[i * info.Channels + info.GStride];
		const float B = in_data[i * info.Channels + info.BStride];

		float x, y, Y;
		srgb_to_xyY(R, G, B, x, y, Y);
		max_luminance = std::max(max_luminance, Y);
	}
#endif

	std::vector<float> out_data;
	out_data.resize(info.Width * info.Height * 3);
	if (max_luminance == 0) {
		std::fill(out_data.begin(), out_data.end(), 0.0f);
	} else {
		for (size_t i = 0; i < info.Width * info.Height; ++i) {
			const float R = in_data[i * info.Channels + info.RStride];
			const float G = in_data[i * info.Channels + info.GStride];
			const float B = in_data[i * info.Channels + info.BStride];
			float& r	  = out_data[i * 3 + 0];
			float& g	  = out_data[i * 3 + 1];
			float& b	  = out_data[i * 3 + 2];

			float x, y, Y;
			srgb_to_xyY(R, G, B, x, y, Y);

			const float L = Y / max_luminance;
			xyY_to_srgb(x, y, reinhard_modified(L), r, g, b);
		}

		if (!options.OutputIsLinear) {
			for (size_t i = 0; i < info.Width * info.Height * 3; ++i) {
				float& c = out_data[i];
				c		 = srgb_from_linear(c);
			}
		}

		if (options.ClampOutput) {
			for (size_t i = 0; i < info.Width * info.Height * 3; ++i) {
				float& c = out_data[i];
				c		 = std::max(0.0f, std::min(1.0f, c));
			}
		}
	}

	if (!write_output(options.OutputFile.generic_string(), out_data, info.Width, info.Height))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
