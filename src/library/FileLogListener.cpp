#include "FileLogListener.h"
#include "config/Build.h"

#include <ctime>
#include <iomanip>
#include <thread>

namespace PR {
FileLogListener::FileLogListener()
	: LogListener()
	, mStream()
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
	mStream << "Build: " << Build::getBuildString() << std::endl;
}

void FileLogListener::startEntry(LogLevel level)
{
	time_t t  = time(0);
	clock_t c = clock();

	struct tm ptm;
#ifndef PR_OS_WINDOWS
	gmtime_r(&t, &ptm);
#else
	gmtime_s(&ptm, &t);
#endif
	mStream << std::setw(2) << std::setfill('0') << ptm.tm_hour
			<< ":" << std::setw(2) << ptm.tm_min << ":"
			<< std::setw(2) << ptm.tm_sec
			<< "(" << c << ")> {"
			<< std::this_thread::get_id() << "} ["
			<< Logger::levelString(level) << "] ";
}

void FileLogListener::writeEntry(int c)
{
	mStream.put(c);
}
} // namespace PR
