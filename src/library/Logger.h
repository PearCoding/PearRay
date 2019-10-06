#pragma once

#include "PR_Config.h"
#include <list>
#include <string>

namespace PR {
enum LogLevel {
	L_DEBUG = 0,
	L_INFO,
	L_WARNING,
	L_ERROR,
	L_FATAL
};

class LogListener;
class PR_LIB Logger {
	PR_CLASS_NON_COPYABLE(Logger);

public:
	class PR_LIB StreamBuf final : public std::streambuf {
	public:
		inline StreamBuf(Logger& logger, bool ignore) : std::streambuf(), mLogger(logger), mIgnore(ignore) {}
		std::streambuf::int_type overflow(std::streambuf::int_type c);
	private:
		Logger& mLogger;
		bool mIgnore;
	};

	Logger();
	~Logger();

	static const char* levelString(LogLevel l);

	void addListener(LogListener* listener);
	void removeListener(LogListener* listener);

	inline void setVerbosity(LogLevel level) { mVerbosity = level; }
	inline LogLevel verbosity() const { return mVerbosity; }

	inline void setQuiet(bool b) { mQuiet = b; }
	inline bool isQuiet() const { return mQuiet; }

	std::ostream& startEntry(LogLevel level);

	static inline Logger& instance()
	{
		static Logger log;
		return log;
	}

	static inline std::ostream& log(LogLevel level) {
		return instance().startEntry(level);
	}

private:
	std::list<LogListener*> mListener;
	LogLevel mVerbosity;
	bool mQuiet;

	StreamBuf mEmptyStreamBuf;
	std::ostream mEmptyStream;

	StreamBuf mStreamBuf;
	std::ostream mStream;
};

PR_LIB std::ostream& operator << (std::ostream& stream, const Vector2f& v);
PR_LIB std::ostream& operator << (std::ostream& stream, const Vector2i& v);
PR_LIB std::ostream& operator << (std::ostream& stream, const Vector3f& v);
PR_LIB std::ostream& operator << (std::ostream& stream, const Eigen::Quaternionf& v);
}

#define PR_LOGGER (PR::Logger::instance())
#define PR_LOG(l) (PR::Logger::log((l)))
