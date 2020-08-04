#pragma once

#include "PR_Config.h"

#include <atomic>
#include <chrono>
#include <filesystem>

namespace PR {
namespace Profiler {
// Structs
struct PR_LIB_BASE EntryDescription {
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

struct PR_LIB_BASE InternalTimeCounter {
	InternalCounter* Total;
	InternalCounter* TimeSpentNS;
};

// Internal interface
void PR_LIB_BASE setThreadName(const std::string& name);
void PR_LIB_BASE emitSignal(const std::string& name);

PR_LIB_BASE InternalCounter* registerCounter(const EntryDescription* desc);
PR_LIB_BASE InternalTimeCounter registerTimeCounter(const EntryDescription* desc);

void PR_LIB_BASE start(uint32 samplesPerSecond, int32 networkPort = -1);
void PR_LIB_BASE stop();

bool PR_LIB_BASE dumpToFile(const std::filesystem::path& filename);
bool PR_LIB_BASE dumpToJSON(const std::filesystem::path& filename);

// Event structure
class PR_LIB_BASE EventScope {
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
		*mCounter.TimeSpentNS += (uint64)std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
	}

private:
	InternalTimeCounter& mCounter;
	std::chrono::high_resolution_clock::time_point mStart;
};

class PR_LIB_BASE Event {
public:
	inline Event(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
		: mDesc(name, function, file, line, 0, category)
		, mCounter(registerTimeCounter(&mDesc))
	{
	}

	inline InternalTimeCounter& counter() { return mCounter; }

private:
	const EntryDescription mDesc;
	InternalTimeCounter mCounter;
};
} // namespace Profiler
} // namespace PR

#define _PR_PROFILE_UNIQUE_NAME_EVENT(line) __profile__##line
#define _PR_PROFILE_UNIQUE_NAME_SCOPE(line) __b_profile__##line

#ifdef PR_WITH_PROFILER
#define PR_PROFILE(name, func, file, line, cat)                                                                               \
	thread_local PR::Profiler::Event _PR_PROFILE_UNIQUE_NAME_EVENT(line)((name), (func), (file), (line), (cat));              \
	const auto _PR_PROFILE_UNIQUE_NAME_SCOPE(line) = PR::Profiler::EventScope(_PR_PROFILE_UNIQUE_NAME_EVENT(line).counter()); \
	PR_UNUSED(_PR_PROFILE_UNIQUE_NAME_SCOPE(line))
#define PR_PROFILE_THREAD(name) PR::Profiler::setThreadName((name))
#define PR_PROFILE_SIGNAL(name) PR::Profiler::emitSignal((name))
#else
#define PR_PROFILE(name, func, file, line, cat) PR_NOOP
#define PR_PROFILE_THREAD(name) PR_NOOP
#define PR_PROFILE_SIGNAL(name) PR_NOOP
#endif

#define PR_PROFILE_THIS PR_PROFILE("", PR_FUNCTION_NAME, __FILE__, __LINE__, "")
