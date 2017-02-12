#include "FileLogListener.h"

#include <ctime>
#include <thread>

namespace PR
{
	FileLogListener::FileLogListener() :
		LogListener(), mStream()
	{
	}

	FileLogListener::~FileLogListener()
	{
		if (mStream.is_open())
			mStream.close();
	}

	void FileLogListener::open(const std::string& file)
	{
		mStream.open(file.c_str(), std::ios::out);
		mStream << "Build: " << PR_BUILD_STRING << std::endl;
	}

	void FileLogListener::newEntry(Level level, Module m, const std::string& str)
	{
		time_t t = time(0);
		clock_t c = clock();

		struct tm ptm;
#ifndef PR_OS_WINDOWS
		gmtime_r(&t, &ptm);
#else
		gmtime_s(&ptm, &t);
#endif
		mStream << ptm.tm_hour << ":" << ptm.tm_min << ":" << ptm.tm_sec
			<< "(" << c << ")> {"
			<< std::this_thread::get_id() << "} ["
			<< Logger::levelString(level) << "] ("
			<< Logger::moduleString(m) << ") "
			<< str << std::endl;
	}
}
