#include "ProgramSettings.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>

#include <iostream>

namespace po = boost::program_options;

po::options_description setup_cmd_options()
{
	po::options_description general_d("Allowed options");
	general_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information into log file (and perhabs into console)")
		("mode,m", "Mode of the sandbox")
	;
	return general_d;
}

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d = setup_cmd_options();
	po::positional_options_description p;
	p.add("mode", -1);

	po::variables_map vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(all_d).positional(p).run(), vm);
		po::notify(vm);
	}
	catch(const std::exception& e)
	{
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}    

	// Handle help
	if (vm.count("help"))
	{
		std::cout << all_d << std::endl;
		exit(1);
	}

	if(!vm.count("mode"))
	{
		std::cout << "No mode given!" << std::endl;
		return false;
	}

	Mode = vm["mode"].as<std::string>();
	IsVerbose = (vm.count("verbose") != 0);
	IsQuiet = (vm.count("quiet") != 0);

	return true;
}