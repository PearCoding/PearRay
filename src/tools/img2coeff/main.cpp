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

#include "spectral/SpectralUpsampler.h"
#include <OpenImageIO/imageio.h>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

class ProgramSettings {
public:
	bf::path InputFile;
	bf::path LookupFile;
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
		("lookup,l", po::value<std::string>(), "Lookup file")
		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>(), "Output file")
		;
	// clang-format on

	po::positional_options_description p;
	p.add("input", 1).add("output", 2);

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
	if (!vm.count("input")) {
		std::cout << "No input given!" << std::endl;
		return false;
	}

	// Input file
	InputFile = vm["input"].as<std::string>();
	if (!bf::exists(InputFile)) {
		std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
		return false;
	}

	if (!vm.count("lookup")) {
		std::cout << "No lookup file given!" << std::endl;
		return false;
	}

	LookupFile = vm["lookup"].as<std::string>();
	if (!bf::exists(LookupFile)) {
		std::cout << "Couldn't find file '" << LookupFile << "'" << std::endl;
		return false;
	}

	if (!vm.count("output")) {
		std::cout << "No output given!" << std::endl;
		return false;
	}
	OutputFile = vm["output"].as<std::string>();

	IsVerbose = (vm.count("verbose") != 0);
	IsQuiet	  = (vm.count("quiet") != 0);

	return true;
}

bool read_input(const std::string& filename, std::vector<float>& data, int& width, int& height)
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
	if (channels != 3) {
		std::cerr << "Error: Expected input file to have three channels" << std::endl;
		return false;
	}

	data.resize(width * height * channels);
	in->read_image(OIIO::TypeDesc::FLOAT, &data[0]);
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
	spec.channelnames.push_back("coeff.A");
	spec.channelnames.push_back("coeff.B");
	spec.channelnames.push_back("coeff.C");
	spec.attribute ("oiio:ColorSpace", "Custom");
	spec.attribute("Software", "PearRay img2coeff tool");

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
		std::cout << "  Input:   " << options.InputFile << std::endl;
		std::cout << "  Output:  " << options.OutputFile << std::endl;
		std::cout << "  Lookup:  " << options.LookupFile << std::endl;
	}

	std::vector<float> in_data;
	int width = 0, height = 0;
	if (!read_input(options.InputFile.generic_string(), in_data, width, height))
		return EXIT_FAILURE;

	std::vector<float> out_data;
	out_data.resize(in_data.size());

	try {
		PR::SpectralUpsampler upsampler(options.LookupFile.generic_wstring());

		std::vector<float> r_buffer(width);
		std::vector<float> g_buffer(width);
		std::vector<float> b_buffer(width);

		std::vector<float> oa_buffer(width);
		std::vector<float> ob_buffer(width);
		std::vector<float> oc_buffer(width);

		for (int i = 0; i < height; ++i) {
			const float* in = &in_data[i * width * 3];
			float* out		= &out_data[i * width * 3];

			PR_OPT_LOOP
			for (int j = 0; j < width; ++j) {
				r_buffer[j] = in[j * 3 + 0];
				g_buffer[j] = in[j * 3 + 1];
				b_buffer[j] = in[j * 3 + 2];
			}

			upsampler.prepare(&r_buffer[0], &g_buffer[0], &b_buffer[0], &oa_buffer[0], &ob_buffer[0], &oc_buffer[0], width);

			PR_OPT_LOOP
			for (int j = 0; j < width; ++j) {
				out[j * 3 + 0] = oa_buffer[j];
				out[j * 3 + 1] = ob_buffer[j];
				out[j * 3 + 2] = oc_buffer[j];
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (!write_output(options.OutputFile.generic_string(), out_data, width, height))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
