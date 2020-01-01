#include "ProgramSettings.h"
#include "Build.h"
#include "EnumOption.h"
#include "cache/Cache.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

using namespace PR;
namespace po = boost::program_options;
namespace bf = boost::filesystem;

constexpr PR::uint32 DEF_THREAD_COUNT  = 0;
constexpr PR::uint32 DEF_THREAD_TILE_X = 8;
constexpr PR::uint32 DEF_THREAD_TILE_Y = 8;

po::options_description setup_cmd_options()
{
	// clang-format off
	po::options_description general_d("General");
	general_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("version", "Show version and exit")
		("verbose,v", "Print detailed information into log file (and perhabs into console)")
#ifdef PR_WITH_PROFILER
		("profile,P", "Profile execution and dump results into a file")
#endif
		("progress,p", po::value<PR::uint32>()->implicit_value(1),
			"Show progress (regardless if quiet or not)")
		("information,I", "Print additional scene information into log file (and perhabs into console)")
		("registry", "Shows the content of the internal registry")

		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>()->default_value("./scene"), "Output directory")
		("pluginpath", po::value<std::string>(), "Additional plugin path")
	;

	po::options_description image_d("Image");
	image_d.add_options()
		("img-update", po::value<float>()->default_value(0.0f),
			"Update interval in seconds where image will be saved. 0 disables it.")
		("img-ext", po::value<std::string>()->default_value("png"),
			"File extension for image output. Has to be a type supported by OpenImageIO.")
	;

	po::options_description thread_d("Threading[*]");
	thread_d.add_options()
		("threads,t", po::value<PR::uint32>(),
			"Amount of threads used for processing. Set 0 for automatic detection.")
		("rtx", po::value<PR::uint32>(),
			"Amount of horizontal tiles used in threading")
		("rty", po::value<PR::uint32>(),
			"Amount of vertical tiles used in threading")
	;

	po::options_description scene_d("Scene[*]");
	scene_d.add_options()
		("itx", po::value<PR::uint32>(),
			"Amount of horizontal image tiles used in rendering")
		("ity", po::value<PR::uint32>(),
			"Amount of vertical image tiles used in rendering")
	;

	po::options_description cache_d("Cache[*]");
	cache_d.add_options()
		("force-cache",
			"Use cache for everything")
		("no-cache",
			"Do not use cache")
	;
	// clang-format on

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
	all_d.add(image_d);
	all_d.add(thread_d);
	all_d.add(scene_d);
	all_d.add(cache_d);

	return all_d;
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
		std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki/PearRayCLI\n"
				  << std::endl;
		std::cout << all_d << std::endl;
		exit(0);
	}

	// Handle version
	if (vm.count("version")) {
		std::cout << Build::getBuildString() << std::endl;
		exit(0);
	}

	// Defaults
	ImageTileXCount = 1;
	ImageTileYCount = 1;

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

	if (!vm.count("output")) {
		std::cout << "No output given!" << std::endl;
		return false;
	}

	PluginPath = "";
	if (vm.count("pluginpath")) {
		PluginPath = vm["pluginpath"].as<std::string>();

		if (!bf::is_directory(PluginPath)) {
			std::cout << "Given plugin path '" << PluginPath
					  << "' is not a valid directory" << std::endl;
			return false;
		}
	}

	// Setup output directory
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

	IsVerbose		= (vm.count("verbose") != 0);
	IsQuiet			= (vm.count("quiet") != 0);
	ShowProgress	= vm.count("progress") ? vm["progress"].as<PR::uint32>() : 0;
	ShowInformation = (vm.count("information") != 0);
	ShowRegistry	= (vm.count("registry") != 0);

#ifdef PR_WITH_PROFILER
	Profile = (vm.count("profile") != 0);
#else
	Profile = false;
#endif

	// Image
	ImgUpdate = vm["img-update"].as<float>();
	ImgExt	= vm["img-ext"].as<std::string>();

	// Thread
	if (vm.count("rtx"))
		RenderTileXCount = vm["rtx"].as<PR::uint32>();
	else if (!vm.count("config"))
		RenderTileXCount = DEF_THREAD_TILE_X;

	if (vm.count("rty"))
		RenderTileYCount = vm["rty"].as<PR::uint32>();
	else if (!vm.count("config"))
		RenderTileYCount = DEF_THREAD_TILE_Y;

	if (vm.count("threads"))
		ThreadCount = vm["threads"].as<PR::uint32>();
	else if (!vm.count("config"))
		ThreadCount = DEF_THREAD_COUNT;

	if (vm.count("itx"))
		ImageTileXCount = std::max<uint32>(1, vm["itx"].as<PR::uint32>());
	if (vm.count("ity"))
		ImageTileYCount = std::max<uint32>(1, vm["ity"].as<PR::uint32>());

	if (vm.count("no-cache"))
		CacheMode = CM_None;
	else if (vm.count("force-cache"))
		CacheMode = CM_All;
	else
		CacheMode = CM_Auto;

	return true;
}
