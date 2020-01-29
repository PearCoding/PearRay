#include "FileLogListener.h"
#include "Logger.h"
#include "ProgramSettings.h"

#include <boost/filesystem.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace bf = boost::filesystem;
namespace sc = std::chrono;

typedef void (*SuiteCallback)();

extern void suite_principled();
extern void suite_projection();
extern void suite_spectral();
extern void suite_random();
extern void suite_material();
extern void suite_triangle();

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
	{ "triangle", suite_triangle },
	{ nullptr, nullptr }
};

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

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "pr_sandbox_" << t << "_d.log";
#else
	sstream << "pr_sandbox_" << t << ".log";
#endif
	const bf::path logFile = sstream.str();

	PR::FileLogListener fileLogListener;
	fileLogListener.open(logFile.string());
	PR_LOGGER.addListener(&fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.setVerbosity(options.IsVerbose ? PR::L_DEBUG : PR::L_INFO);

	// Create results directory
	if (!bf::exists("results") || !bf::is_directory("results"))
		bf::create_directory("results");

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

	return 0;
}
