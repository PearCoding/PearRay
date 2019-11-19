#pragma once

#include "PR_Config.h"

#include <atomic>
#include <chrono>

namespace PR {
namespace Profiler {
// Structs
struct PR_LIB EntryDescription {
	std::string Name;
	std::string Function;
	std::string File;
	uint32 Line;
	uint32 ThreadID;
	std::string Category;

	inline EntryDescription(const std::string& name, const std::string& function,
							const std::string& file, uint32 line, uint32 threadID,
							const std::string& category)
		: Name(name)
		, Function(function)
		, File(file)
		, Line(line)
		, ThreadID(threadID)
		, Category(category)
	{
	}
};
typedef std::atomic<uint64> InternalCounter;

struct PR_LIB InternalTimeCounter {
	InternalCounter* Total;
	InternalCounter* TimeSpentMicroSec;
};

// Internal interface
void PR_LIB setThreadName(const std::string& name);

PR_LIB InternalCounter* registerCounter(const EntryDescription* desc);
PR_LIB InternalTimeCounter registerTimeCounter(const EntryDescription* desc);

void PR_LIB start(uint32 samplesPerSecond, int32 networkPort = -1);
void PR_LIB stop();

bool PR_LIB dumpToFile(const std::wstring& filename);

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
		*mCounter.TimeSpentMicroSec += (uint64)std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
	}

private:
	InternalTimeCounter& mCounter;
	std::chrono::high_resolution_clock::time_point mStart;
};

class PR_LIB Event {
public:
	inline Event(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
		: mDesc(name, function, file, line, 0, category)
		, mCounter(registerTimeCounter(&mDesc))
	{
	}

	inline EventScope scope()
	{
		return EventScope(mCounter);
	}

private:
	const EntryDescription mDesc;
	InternalTimeCounter mCounter;
};
} // namespace Profiler
} // namespace PR

#define _PR_PROFILE_UNIQUE_NAME(line) __profile__##line

#ifdef PR_WITH_PROFILER
#define PR_PROFILE(name, func, file, line, cat)                                                            \
	thread_local PR::Profiler::Event _PR_PROFILE_UNIQUE_NAME(line)((name), (func), (file), (line), (cat)); \
	_PR_PROFILE_UNIQUE_NAME(line).scope()
#define PR_PROFILE_THREAD(name) PR::Profiler::setThreadName((name))
#else
#define PR_PROFILE(name, func, file, line, cat)
#define PR_PROFILE_THREAD(name)
#endif

#define PR_PROFILE_THIS PR_PROFILE("", PR_FUNCTION_NAME, __FILE__, __LINE__, "")
