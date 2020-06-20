#include "ProgramSettings.h"

#include <cxxopts.hpp>
#include <iostream>

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("prsandbox", "Sandbox environment");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("help-mode", "List all available modes")
			("q,quiet", "Do not print messages into console")
			("v,verbose", "Print detailed information into log file (and perhabs into console)")
			("no-profile", "Disable profiling")
			("m,mode", "Mode of the sandbox")
		;
		// clang-format on
		options.parse_positional({ "mode" });

		cxxopts::ParseResult vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << options.help() << std::endl;
			exit(0);
		}

		Mode		= (vm.count("mode") != 0) ? vm["mode"].as<std::string>() : "";
		HelpMode	= (vm.count("help-mode") != 0);
		IsVerbose	= (vm.count("verbose") != 0);
		IsQuiet		= (vm.count("quiet") != 0);
		NoProfiling = (vm.count("no-profile") != 0);
	} catch (const cxxopts::OptionException& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	return true;
}
