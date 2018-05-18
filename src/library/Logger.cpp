#include "Logger.h"
#include "LogListener.h"

#include <stdarg.h>
#include <stdio.h>
#include <string>

namespace PR {
Logger::Logger()
#ifdef PR_DEBUG
	: mVerbosity(L_DEBUG)
#else
	: mVerbosity(L_INFO)
#endif
	, mQuiet(false)
	, mEmptyStreamBuf(*this, true)
	, mEmptyStream(&mEmptyStreamBuf)
	, mStreamBuf(*this, false)
	, mStream(&mStreamBuf)
{
}

Logger::~Logger()
{
}

const char* levelStr[] = {
	"Debug  ",
	"Info   ",
	"Warning",
	"Error  ",
	"Fatal  "
};

const char* Logger::levelString(LogLevel l)
{
	return levelStr[l];
}

void Logger::addListener(LogListener* listener)
{
	mListener.push_back(listener);
}

void Logger::removeListener(LogListener* listener)
{
	mListener.remove(listener);
}

std::ostream& Logger::startEntry(LogLevel level) {
	if ((int)verbosity() < (int)level)
		return mEmptyStream;

	if (!mQuiet)
		std::cout << "[" << levelStr[level] << "] ";

	for (std::list<LogListener*>::iterator it = mListener.begin();
		 it != mListener.end();
		 ++it) {
		(*it)->startEntry(level);
	}

	return mStream;
}

std::streambuf::int_type Logger::StreamBuf::overflow(std::streambuf::int_type c) {
	if (mIgnore)
		return 0;
		
	if (!mLogger.isQuiet())
		std::cout.put(c);

	for (std::list<LogListener*>::iterator it = mLogger.mListener.begin();
		 it != mLogger.mListener.end();
		 ++it) {
		(*it)->writeEntry(c);
	}

	return 0;
}

std::ostream& operator << (std::ostream& stream, const Eigen::Vector2f& v) {
	stream << v.x() << ", " << v.y();
	return stream;
}

std::ostream& operator << (std::ostream& stream, const Eigen::Vector2i& v) {
	stream << v.x() << ", " << v.y();
	return stream;
}

std::ostream& operator << (std::ostream& stream, const Eigen::Vector3f& v) {
	stream << v.x() << ", " << v.y() << ", " << v.z();
	return stream;
}

std::ostream& operator << (std::ostream& stream, const Eigen::Quaternionf& v) {
	stream << v.w() << ", " << v.x() << ", " << v.y() << ", " << v.z();
	return stream;
}

}
