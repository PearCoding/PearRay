#pragma once

#include "PR_Config.h"

#include <atomic>
#include <chrono>

namespace PR {
namespace Profiler {
// Internal interface
void PR_LIB setThreadName(const std::string& name);

typedef std::atomic<uint64> InternalCounter;
InternalCounter* registerCounter(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category);

struct InternalTimeCounter {
	InternalCounter* Total;
	InternalCounter* TimeSpentMicroSec;
};
InternalTimeCounter registerTimeCounter(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category);

void PR_LIB start(uint32 samplesPerSecond, int32 networkPort = -1);
void PR_LIB stop();

void PR_LIB dumpToFile(const std::wstring& filename);

// Event structure
class PR_LIB EventScope {
public:
	inline explicit EventScope(InternalTimeCounter& counter)
		: mCounter(counter)
		, mStart(std::chrono::high_resolution_clock::now())
	{
	}

	inline ~EventScope()
	{
		const auto dur = std::chrono::high_resolution_clock::now() - mStart;
		++(*mCounter.Total);
		*mCounter.TimeSpentMicroSec += std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
	}

private:
	InternalTimeCounter& mCounter;
	std::chrono::high_resolution_clock::time_point mStart;
};

class PR_LIB Event {
public:
	inline Event(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
		: mCounter(registerTimeCounter(name, function, file, line, category))
	{
	}

	inline EventScope scope()
	{
		return EventScope(mCounter);
	}

private:
	InternalTimeCounter mCounter;
};
} // namespace Profiler
} // namespace PR

#define _PR_PROFILE_UNIQUE_NAME(line) __profile__##line

#ifdef PR_WITH_PROFILER
#define PR_PROFILE(name, func, file, line, cat)                                                  \
	thread_local PR::Profiler::Event _PR_PROFILE_UNIQUE_NAME(line)(name, func, file, line, cat); \
	_PR_PROFILE_UNIQUE_NAME(line).scope()
#else
#define PR_PROFILE(name, func, file, line, cat)
#endif

#define PR_PROFILE_THIS PR_PROFILE("", PR_FUNCTION_NAME, __FILE__, __LINE__, "")
