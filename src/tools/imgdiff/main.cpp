#include <boost/filesystem.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <OpenImageIO/imageio.h>

#include "PR_Config.h"

namespace po = boost::program_options;
namespace bf = boost::filesystem;

class ProgramSettings {
public:
	bf::path InputFile1;
	bf::path InputFile2;
	bf::path OutputFile;

	bool IsVerbose;
	bool IsQuiet;

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d("Allowed Options");

	// clang-format off
    all_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information")
		("input1", po::value<std::string>(), "Input1 file")
		("input2", po::value<std::string>(), "Input2 file")
		("output,o", po::value<std::string>(), "Output file")
		;
	// clang-format on

	po::positional_options_description p;
	p.add("input1", 1).add("input2", 1).add("output", 1);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(all_d).positional(p).run(), vm);
		po::notify(vm);
	} catch (const std::exception& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	// Handle help
	if (vm.count("help")) {
		std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
				  << std::endl;
		std::cout << all_d << std::endl;
		exit(0);
	}

	// Handle version
	if (!vm.count("input1") || !vm.count("input2")) {
		std::cout << "No input given!" << std::endl;
		return false;
	}

	// Input file 1
	InputFile1 = vm["input1"].as<std::string>();
	if (!bf::exists(InputFile1)) {
		std::cout << "Couldn't find file '" << InputFile1 << "'" << std::endl;
		return false;
	}

	// Input file 2
	InputFile2 = vm["input2"].as<std::string>();
	if (!bf::exists(InputFile2)) {
		std::cout << "Couldn't find file '" << InputFile2 << "'" << std::endl;
		return false;
	}

	// Output file
	if (!vm.count("output")) {
		std::cout << "No output given!" << std::endl;
		return false;
	}
	OutputFile = vm["output"].as<std::string>();

	IsVerbose = (vm.count("verbose") != 0);
	IsQuiet	  = (vm.count("quiet") != 0);

	return true;
}

bool read_input(const std::string& filename, std::vector<float>& data, int& width, int& height, int& channels)
{
	auto in = OIIO::ImageInput::open(filename);
	if (!in) {
		std::cerr << "Error: Could not open input file" << std::endl;
		return false;
	}
	const OIIO::ImageSpec& spec = in->spec();
	width						= spec.width;
	height						= spec.height;
	channels					= std::min(3, spec.nchannels);
	if (spec.nchannels > 3)
		std::cerr << "Warning: More than 3 channels are not supported" << std::endl;

	data.resize(width * height * channels);
	in->read_image(0, channels, OIIO::TypeDesc::FLOAT, &data[0]);
	in->close();

	return true;
}

bool write_output(const std::string& filename, const std::vector<float>& data, int width, int height, int channels)
{
	auto out = OIIO::ImageOutput::create(filename);
	if (!out) {
		std::cerr << "Error: Could not create output file" << std::endl;
		return false;
	}

	OIIO::ImageSpec spec(width, height, channels, OIIO::TypeDesc::FLOAT);
	spec.channelnames.clear();
	if (channels == 1)
		spec.channelnames.push_back("A");
	else {
		spec.channelnames.push_back("R");
		spec.channelnames.push_back("G");
		spec.channelnames.push_back("B");
	}
	spec.attribute("oiio:ColorSpace", "Custom");
	spec.attribute("Software", "PearRay imgdiff tool");

	out->open(filename, spec);
	out->write_image(OIIO::TypeDesc::FLOAT, &data[0]);
	out->close();

	return true;
}

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	if (!options.IsQuiet && options.IsVerbose) {
		std::cout << "Arguments> " << std::endl;
		std::cout << "  Input1: " << options.InputFile1 << std::endl;
		std::cout << "  Input2: " << options.InputFile2 << std::endl;
		std::cout << "  Output: " << options.OutputFile << std::endl;
	}

	std::vector<float> in_data1;
	int width1 = 0, height1 = 0, channels1 = 0;
	if (!read_input(options.InputFile1.generic_string(), in_data1, width1, height1, channels1))
		return EXIT_FAILURE;

	std::vector<float> in_data2;
	int width2 = 0, height2 = 0, channels2 = 0;
	if (!read_input(options.InputFile2.generic_string(), in_data2, width2, height2, channels2))
		return EXIT_FAILURE;

	if (width1 != width2 || height1 != height2 || channels1 != channels2) {
		std::cerr << "Two inputs does not match in shape" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<float> out_data;
	out_data.resize(in_data1.size());

	PR_OPT_LOOP
	for (size_t i = 0; i < in_data1.size(); ++i) {
		out_data[i] = std::abs(in_data1[i] - in_data2[i]);
	}

	if (!write_output(options.OutputFile.generic_string(), out_data, width1, height1, channels1))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
