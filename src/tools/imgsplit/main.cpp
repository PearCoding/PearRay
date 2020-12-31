#include <cmath>
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include <OpenImageIO/imageio.h>

namespace sf = std::filesystem;
bool read_input(const std::string& filename, std::vector<float>& data, int& width, int& height, std::vector<std::string>& channel_names);

enum CropMode {
	CM_None = 0,
	CM_Pixel,
	CM_Normalized
};

class ProgramSettings {
public:
	sf::path InputFile;
	sf::path OutputFile;

	bool IsVerbose;
	std::vector<std::string> Channels;

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("prsplit", "Split multi channel image");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("l,list", "List all available channels and exit")
			("v,verbose", "Print detailed information")
			("c,channels", "List of channels to split if available", cxxopts::value<std::vector<std::string>>())
			("i,input", "Input file", cxxopts::value<std::string>())
			("o,output", "Output file", cxxopts::value<std::string>())
		;
		// clang-format on
		options.parse_positional({ "input", "output" });

		auto vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
					  << std::endl;
			std::cout << options.help() << std::endl;
			exit(EXIT_SUCCESS);
		}

		if (vm.count("list")) {
			if (!vm.count("input")) {
				std::cerr << "Error: No input given!" << std::endl;
				return false;
			}

			std::vector<std::string> channel_names;
			std::vector<float> in_data;
			int width = 0, height = 0;
			bool res = read_input(vm["input"].as<std::string>(), in_data, width, height, channel_names);
			if (!res)
				return false;

			for (size_t i = 0; i < channel_names.size(); ++i)
				std::cout << channel_names[i] << std::endl;

			exit(EXIT_SUCCESS);
		}

		// Handle version
		if (!vm.count("input") || !vm.count("output")) {
			std::cerr << "Error: No input and output given!" << std::endl;
			return false;
		}

		// Input file
		InputFile = vm["input"].as<std::string>();
		if (!sf::exists(InputFile)) {
			std::cerr << "Error: Couldn't find file '" << InputFile << "'" << std::endl;
			return false;
		}

		// Output file
		OutputFile = vm["output"].as<std::string>();

		if (vm.count("channels"))
			Channels = vm["channels"].as<std::vector<std::string>>();
		else
			Channels = { "R", "G", "B" };

		IsVerbose = (vm.count("verbose") != 0);
	} catch (const cxxopts::OptionException& e) {
		std::cerr << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool read_input(const std::string& filename, std::vector<float>& data, int& width, int& height, std::vector<std::string>& channel_names)
{
	auto in = OIIO::ImageInput::open(filename);
	if (!in) {
		std::cerr << "Error: Could not open input file" << std::endl;
		return false;
	}
	const OIIO::ImageSpec& spec = in->spec();
	width						= spec.width;
	height						= spec.height;
	int channels				= spec.nchannels;

	channel_names.resize(channels);
	for (int i = 0; i < channels; ++i)
		channel_names[i] = spec.channel_name(i);

	data.resize(width * height * channels);
	in->read_image(0, channels, OIIO::TypeDesc::FLOAT, &data[0]);
	in->close();

	return true;
}

struct ChannelInfo {
	std::string Name;
	size_t Stride;
};

bool write_output(const std::string& filename, const std::vector<float>& data, int width, int height, const std::vector<ChannelInfo>& channels, size_t in_channel_count)
{
	auto out = OIIO::ImageOutput::create(filename);
	if (!out) {
		std::cerr << "Error: Could not create output file" << std::endl;
		return false;
	}

	const int channel_count = (int)channels.size();
	OIIO::ImageSpec spec(width, height, channel_count, OIIO::TypeDesc::FLOAT);
	spec.channelnames.resize(channel_count);
	for (int i = 0; i < channel_count; ++i)
		spec.channelnames[i] = channels[i].Name;
	spec.attribute("Software", "PearRay imgsplit tool");

	std::vector<float> line(width * channel_count);

	out->open(filename, spec);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			for (int c = 0; c < channel_count; ++c)
				line[x * channel_count + c] = data[y * width * in_channel_count + x * in_channel_count + channels[c].Stride];
		}
		out->write_scanline(y, 0, OIIO::TypeDesc::FLOAT, &line[0]);
	}
	out->close();

	return true;
}

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	if (options.IsVerbose) {
		std::cout << "Arguments> " << std::endl;
		std::cout << "  Input:  " << options.InputFile << std::endl;
		std::cout << "  Output: " << options.OutputFile << std::endl;
	}

	std::vector<std::string> channel_names;
	std::vector<float> in_data;
	int width = 0, height = 0;
	if (!read_input(options.InputFile.generic_string(), in_data, width, height, channel_names))
		return EXIT_FAILURE;

	if (options.IsVerbose) {
		std::cout << "Available channels> " << std::endl;
		for (size_t i = 0; i < channel_names.size(); ++i)
			std::cout << "  " << channel_names[i] << std::endl;
	}

	std::vector<ChannelInfo> channel_info;
	for (size_t i = 0; i < channel_names.size(); ++i) {
		auto it = std::find(options.Channels.begin(), options.Channels.end(), channel_names[i]);
		if (it == options.Channels.end())
			continue;

		channel_info.push_back(ChannelInfo{ channel_names[i], i });
	}

	if (channel_info.empty()) {
		std::cerr << "Error: Could not find the channels to output" << std::endl;
		return EXIT_FAILURE;
	}

	if (!write_output(options.OutputFile.string(), in_data, width, height, channel_info, channel_names.size()))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
