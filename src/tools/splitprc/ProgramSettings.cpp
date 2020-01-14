#include "ProgramSettings.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

constexpr size_t DEF_MIN_SIZE_KB = 1024 * 64; // 64 Mb
po::options_description setup_cmd_options()
{
	// clang-format off
	po::options_description general_d("Allowed Options");
	general_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information")
		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>(), "Output directory")
		("overwrite,f", "Overwrite input file")
		("no-include", "Do not generate file with includes")
		("size,s", po::value<size_t>()->default_value(DEF_MIN_SIZE_KB), "Minimum size of generated chunks. If input is smaller, nothing will be done")
	;

    return general_d;
}

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d = setup_cmd_options();
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

	// Setup output directory
    if(vm.count("output")) {
	    const bf::path relativePath = vm["output"].as<std::string>();
	    if (!bf::exists(relativePath)) {
		    if (!bf::create_directory(relativePath)) {
			    std::cout << "Couldn't create directory '" << relativePath << "'" << std::endl;
			    return false;
		    }
	    }

	    const bf::path directoryPath = relativePath.is_relative() ? bf::canonical(relativePath, bf::current_path()) : relativePath;
	    if (!bf::is_directory(directoryPath)) {
		    std::cout << "Invalid output path given." << std::endl;
		    return false;
	    }
	    OutputDir = directoryPath;
    }
    else {
        OutputDir = bf::path(InputFile).parent_path();
    }

	IsVerbose		= (vm.count("verbose") != 0);
	IsQuiet			= (vm.count("quiet") != 0);
	OverwriteInput = (vm.count("overwrite") != 0);
    NoIncludeGenerator = (vm.count("no-include") != 0);

    MinSizeKb = vm["size"].as<size_t>();

	return true;
}
