#include "ProgramSettings.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

namespace po = boost::program_options;

po::options_description setup_cmd_options()
{
	po::options_description general_d("Allowed options");
	// clang-format off
	general_d.add_options()
		("help,h", "Produce this help message")
		("help-mode", "List all available modes")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information into log file (and perhabs into console)")
		("no-profile", "Disable profiling")
		("mode,m", "Mode of the sandbox")
	;
	// clang-format on
	return general_d;
}

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d = setup_cmd_options();
	po::positional_options_description p;
	p.add("mode", -1);

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
		std::cout << all_d << std::endl;
		exit(1);
	}

	Mode		= (vm.count("mode") != 0) ? vm["mode"].as<std::string>() : "";
	HelpMode	= (vm.count("help-mode") != 0);
	IsVerbose	= (vm.count("verbose") != 0);
	IsQuiet		= (vm.count("quiet") != 0);
	NoProfiling = (vm.count("no-profile") != 0);

	return true;
}
