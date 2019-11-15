#include "Profiler.h"

#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace PR {
namespace Profiler {
struct EntryDescription {
	std::string Name;
	std::string Function;
	std::string File;
	uint32 Line;
	std::string Category;

	inline EntryDescription(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
		: Name(name)
		, Function(function)
		, File(file)
		, Line(line)
		, Category(category)
	{
	}
};

struct CounterEntry {
	Profiler::InternalCounter* CounterPtr;
	EntryDescription Desc;

	inline CounterEntry(Profiler::InternalCounter* counter, const EntryDescription& desc)
		: CounterPtr(counter)
		, Desc(desc)
	{
	}
};

struct TimeCounterEntry {
	Profiler::InternalCounter* TotalCounterPtr;
	Profiler::InternalCounter* TimeSpentCounterPtr;
	EntryDescription Desc;

	inline TimeCounterEntry(Profiler::InternalCounter* totalCounter,
					 Profiler::InternalCounter* timeSpentCounter,
					 const EntryDescription& desc)
		: TotalCounterPtr(totalCounter)
		, TimeSpentCounterPtr(timeSpentCounter)
		, Desc(desc)
	{
	}
};

struct ThreadData {
	std::string Name;
	std::vector<CounterEntry> CounterEntries;
	std::vector<TimeCounterEntry> TimeCounterEntries;
	std::vector<std::shared_ptr<Profiler::InternalCounter>> InternalCounters;

	inline explicit ThreadData(const std::string& name)
		: Name(name)
	{
	}
};

static std::mutex sThreadMutex;
static std::vector<ThreadData> sThreadData;

ThreadData* getCurrentThreadData()
{
	thread_local int64 id = -1;
	if (id < 0) {
		std::stringstream stream;
		stream << "Thread " << std::this_thread::get_id();

		id = static_cast<int64>(sThreadData.size());
		sThreadData.emplace_back(stream.str());

		return &sThreadData[id];
	} else {
		return &sThreadData[id];
	}
}

void setThreadName(const std::string& name)
{
	std::lock_guard<std::mutex> guard(sThreadMutex);
	getCurrentThreadData()->Name = name;
}

InternalCounter* registerCounter(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
{
	std::lock_guard<std::mutex> guard(sThreadMutex);
	ThreadData* data = getCurrentThreadData();

	auto counter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(counter);

	data->CounterEntries.emplace_back(counter.get(), EntryDescription(name, function, file, line, category));
	return counter.get();
}

InternalTimeCounter registerTimeCounter(const std::string& name, const std::string& function, const std::string& file, uint32 line, const std::string& category)
{
	std::lock_guard<std::mutex> guard(sThreadMutex);
	ThreadData* data = getCurrentThreadData();

	auto totalCounter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(totalCounter);
	auto timeSpentCounter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(timeSpentCounter);

	data->TimeCounterEntries.emplace_back(totalCounter.get(), timeSpentCounter.get(), EntryDescription(name, function, file, line, category));

	return InternalTimeCounter{ totalCounter.get(), timeSpentCounter.get() };
}

void start(uint32 samplesPerSecond, int32 networkPort)
{
}

void stop()
{
}

void dumpToFile(const std::wstring& filename)
{
}
} // namespace Profiler
} // namespace PR
