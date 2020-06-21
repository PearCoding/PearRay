#include "log/FileLogListener.h"
#include "Logger.h"
#include "Profiler.h"
#include "ProgramSettings.h"

#include <filesystem>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace sf = std::filesystem;
namespace sc = std::chrono;
using namespace PR;

typedef void (*SuiteCallback)();

extern void suite_principled();
extern void suite_projection();
extern void suite_spectral();
extern void suite_random();
extern void suite_material();

struct Suite {
	const char* Name;
	SuiteCallback Callback;
};

Suite suites[] = {
	{ "principled", suite_principled },
	{ "projection", suite_projection },
	{ "random", suite_random },
	{ "spectral", suite_spectral },
	{ "material", suite_material },
	{ nullptr, nullptr }
};

constexpr uint32 PROFILE_SAMPLE_RATE = 100;
int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return -1;

	if (options.HelpMode) {
		std::cout << "Available modes:" << std::endl;
		for (size_t i = 0; suites[i].Name; ++i) {
			std::cout << "  " << suites[i].Name << std::endl;
		}
		return 0;
	}

	if (options.Mode.empty()) {
		std::cerr << "No mode selected. Use --help-mode for a list of available modes." << std::endl;
		return -1;
	}

	if (!options.NoProfiling)
		Profiler::start(PROFILE_SAMPLE_RATE);

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "pr_sandbox_" << t << "_d.log";
#else
	sstream << "pr_sandbox_" << t << ".log";
#endif
	const sf::path logFile = sstream.str();

	std::shared_ptr<FileLogListener> fileLogListener = std::make_shared<FileLogListener>();
	fileLogListener->open(logFile.string());
	PR_LOGGER.addListener(fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.setVerbosity(options.IsVerbose ? L_DEBUG : L_INFO);

	if (!options.NoProfiling) {
		PR_LOG(L_INFO) << "Profiling enabled (TpS: "
					   << std::chrono::high_resolution_clock::period::den
					   << ")" << std::endl;
	}

	// Create results directory
	if (!sf::exists("results") || !sf::is_directory("results"))
		sf::create_directory("results");

	SuiteCallback callback = nullptr;
	for (int i = 0; suites[i].Name; ++i) {
		if (options.Mode == suites[i].Name) {
			callback = suites[i].Callback;
			break;
		}
	}

	if (callback) {
		callback();
	} else {
		std::cout << "Unknown Mode given. Should be one of: " << std::endl
				  << "   ";
		for (int i = 0; suites[i].Name; ++i) {
			std::cout << suites[i].Name << " ";
		}
		std::cout << std::endl;
	}

	if (!options.NoProfiling) {
		Profiler::stop();
		const sf::path profFile = "pr_profile.prof";
		if (!Profiler::dumpToFile(profFile.generic_wstring()))
			PR_LOG(L_ERROR) << "Could not write profile data to " << profFile << std::endl;
		const sf::path profJSONFile = "pr_profile.json";
		if (!Profiler::dumpToJSON(profJSONFile.generic_wstring()))
			PR_LOG(L_ERROR) << "Could not write profile data to " << profJSONFile << std::endl;
	}

	return 0;
}
