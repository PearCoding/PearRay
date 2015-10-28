#include <sstream>

#include "FileLogListener.h"

// PearPic
#include "ExtensionManager.h"

int main(int argc, char** argv)
{
	// Init logging system
	PR::FileLogListener fileLog;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << t << "_d.log";
#else
	sstream << t << ".log";
#endif
	fileLog.open(sstream.str());
	PR_LOGGER.addListener(&fileLog);

#ifdef PR_DEBUG
	PR_LOGGER.setVerbose(true);
#endif

	// Init PearPic extensions
	PP::ExtensionManager::init();

	// TODO

	// Release PearPic extensions
	PP::ExtensionManager::exit();
	return 0;
}