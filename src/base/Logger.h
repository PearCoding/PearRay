#pragma once

#include "PrettyPrint.h"
#include <streambuf>
#include <vector>

namespace PR {
enum LogLevel {
	L_DEBUG = 0,
	L_INFO,
	L_WARNING,
	L_ERROR,
	L_FATAL
};

class LogListener;
class ConsoleLogListener;
class PR_LIB_BASE Logger {
	PR_CLASS_NON_COPYABLE(Logger);

public:
	class PR_LIB_BASE StreamBuf final : public std::streambuf {
	public:
		inline StreamBuf(Logger& logger, bool ignore)
			: std::streambuf()
			, mLogger(logger)
			, mIgnore(ignore)
		{
		}
		std::streambuf::int_type overflow(std::streambuf::int_type c);

	private:
		Logger& mLogger;
		bool mIgnore;
	};

	Logger();
	~Logger();

	static const char* levelString(LogLevel l);

	void addListener(const std::shared_ptr<LogListener>& listener);
	void removeListener(const std::shared_ptr<LogListener>& listener);

	inline void setVerbosity(LogLevel level) { mVerbosity = level; }
	inline LogLevel verbosity() const { return mVerbosity; }

	void setQuiet(bool b);
	inline bool isQuiet() const { return mQuiet; }

	void enableAnsiTerminal(bool b);
	bool isUsingAnsiTerminal() const;

	std::ostream& startEntry(LogLevel level);

	static inline Logger& instance()
	{
		static Logger log;
		return log;
	}

	static inline std::ostream& log(LogLevel level)
	{
		return instance().startEntry(level);
	}

private:
	std::vector<std::shared_ptr<LogListener>> mListener;
	std::shared_ptr<ConsoleLogListener> mConsoleLogListener;

	LogLevel mVerbosity;
	bool mQuiet;

	StreamBuf mEmptyStreamBuf;
	std::ostream mEmptyStream;

	StreamBuf mStreamBuf;
	std::ostream mStream;
};
} // namespace PR

#define PR_LOGGER (PR::Logger::instance())
#define PR_LOG(l) (PR::Logger::log((l)))
