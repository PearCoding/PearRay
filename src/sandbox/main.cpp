#include "Logger.h"
#include "FileLogListener.h"
#include "ProgramSettings.h"

#include <boost/filesystem.hpp>

#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace bf = boost::filesystem;
namespace sc = std::chrono;

typedef void (*SuiteCallback)();

void suite_projection1();
void suite_spectral1();
void suite_random1();

struct Suite
{
	const char* Name;
	SuiteCallback Callback;
};

Suite suites[] = {
	{"projection1", suite_projection1},
	{"spectral1", suite_spectral1},
	{"random1", suite_random1},
	{nullptr, nullptr}
};

int main(int argc, char** argv)
{
	ProgramSettings options;
	if(!options.parse(argc, argv))
		return -1;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "pr_sandbox_" << t << "_d.log";
#else
	sstream << "pr_sandbox_" << t << ".log";
#endif
	const bf::path logFile = sstream.str();

	PR::FileLogListener fileLogListener;
	fileLogListener.open(logFile.native());
	PR_LOGGER.addListener(&fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.setVerbosity(options.IsVerbose ? PR::L_DEBUG : PR::L_INFO);

	if(!options.IsQuiet)
		std::cout << PR_NAME_STRING << " " << PR_VERSION_STRING << " (C) "  << PR_VENDOR_STRING << std::endl;

	// Create results directory
	if(!bf::exists("results") || !bf::is_directory("results"))
		bf::create_directory("results");

	SuiteCallback callback = nullptr;
	for(int i = 0; suites[i].Name; ++i)
	{
		if(options.Mode == suites[i].Name)
		{
			callback = suites[i].Callback;
			break;
		}
	}

	if(callback)
	{
		callback();
	}
	else
	{
		std::cout << "Unknown Mode given. Should be one of: " << std::endl << "   ";
		for(int i = 0; suites[i].Name; ++i)
		{
			std::cout << suites[i].Name << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}
