#include "ProgramSettings.h"
#include "EnumOption.h"
#include "cache/Cache.h"
#include "config/Build.h"

#include <cxxopts.hpp>
#include <iostream>

using namespace PR;
namespace sf = std::filesystem;

constexpr PR::uint32 DEF_THREAD_COUNT  = 0;
constexpr PR::uint32 DEF_THREAD_TILE_X = 8;
constexpr PR::uint32 DEF_THREAD_TILE_Y = 8;

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("pearray", "An experimental high accurate spectral path and ray tracer for research and data acquisition");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("q,quiet", "Do not print messages into console")
			("no-pretty-console", "Do not use decorations to make console output better")
			("version", "Show version and exit")
			("v,verbose", "Print detailed information into log file (and perhabs into console)")
			("P,profile", "Profile execution and dump results into a file")
			("p,progress", "Show progress (regardless if quiet or not)", cxxopts::value<PR::uint32>()->implicit_value("1"))
			("I,information", "Print additional scene information into log file (and perhabs into console)")

			("i,input", "Input file", cxxopts::value<std::string>())
			("o,output", "Output directory", cxxopts::value<std::string>()->default_value("./scene"))
			("pluginpath", "Additional plugin path", cxxopts::value<std::string>())

			("max-time", "Maximum time in seconds to spend on image regardless of given sample parameters. 0 disables it.", cxxopts::value<PR::uint32>()->default_value("0"))
			("force-time-stop", "Force the execution to stop after reaching maximum time regardless of finished iterations.")

			("img-update", "Update interval in seconds where image will be periodically saved. 0 disables it.", cxxopts::value<PR::uint32>()->default_value("0"))
			("img-iteration-update", "Update interval in iterations where image will be periodically saved. 0 disables it.", cxxopts::value<PR::uint32>()->default_value("0"))
			("img-use-tags", "Use tags _n to make sure no image produced in the session is replaced by the following one.")

			("no-network", "Disable network support for clients")
			("network-port", "Set port to listen on", cxxopts::value<PR::uint16>()->default_value("4217"))

			("t,threads", "Amount of threads used for processing. Set 0 for automatic detection.", cxxopts::value<PR::uint32>())
			("rtx", "Amount of horizontal tiles used in threading", cxxopts::value<PR::uint32>())
			("rty", "Amount of vertical tiles used in threading", cxxopts::value<PR::uint32>())
			("adaptive-tiling", "Enable adaptive tiling used for better thread workload balance. Will improve performance of complex scenes but makes reproducibility of results impossible")
			("no-hit-sorting", "Disable sorting of hits to improve cache coherence")

			("itx", "Amount of horizontal image tiles used in rendering", cxxopts::value<PR::uint32>())
			("ity", "Amount of vertical image tiles used in rendering", cxxopts::value<PR::uint32>())

			("force-cache", "Use cache for everything")
			("no-cache", "Do not use cache")
		;
		// clang-format on

		options.parse_positional({ "input", "output" });

		auto vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki/PearRayCLI\n"
					  << std::endl;
			std::cout << options.help() << std::endl;
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
		if (!sf::exists(InputFile)) {
			std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
			return false;
		}

		PluginPath = "";
		if (vm.count("pluginpath")) {
			PluginPath = vm["pluginpath"].as<std::string>();

			if (!sf::is_directory(PluginPath)) {
				std::cout << "Given plugin path '" << PluginPath
						  << "' is not a valid directory" << std::endl;
				return false;
			}
		}

		// Setup output directory
		const sf::path relativePath = vm["output"].as<std::string>();
		if (!sf::exists(relativePath)) {
			if (!sf::create_directory(relativePath)) {
				std::cout << "Couldn't create directory '" << relativePath << "'" << std::endl;
				return false;
			}
		}

		const sf::path directoryPath = relativePath.is_relative() ? sf::canonical(relativePath) : relativePath;
		if (!sf::is_directory(directoryPath)) {
			std::cout << "Invalid output path given." << std::endl;
			return false;
		}
		OutputDir = directoryPath;

		IsVerbose		= (vm.count("verbose") != 0);
		IsQuiet			= (vm.count("quiet") != 0);
		NoPrettyConsole = (vm.count("no-pretty-console") != 0);
		ShowProgress	= vm.count("progress") ? vm["progress"].as<PR::uint32>() : 0;
		ShowInformation = (vm.count("information") != 0);

#ifdef PR_WITH_PROFILER
		Profile = (vm.count("profile") != 0);
#else
		Profile = false;
#endif

		// Timing
		MaxTime		 = vm["max-time"].as<PR::uint32>();
		MaxTimeForce = (vm.count("force-time-stop") != 0);

		// Image
		ImgUpdate		   = vm["img-update"].as<PR::uint32>();
		ImgUpdateIteration = vm["img-iteration-update"].as<PR::uint32>();
		ImgUseTags		   = (vm.count("img-use-tags") != 0);

		// Network
		if (vm.count("no-network"))
			ListenNetwork = -1;
		else
			ListenNetwork = vm["network-port"].as<PR::uint32>();

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

		AdaptiveTiling = (vm.count("adaptive-tiling") != 0);
		SortHits	   = (vm.count("no-hit-sorting") == 0);

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
	} catch (const cxxopts::OptionException& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	return true;
}
