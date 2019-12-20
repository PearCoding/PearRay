#include "Profiler.h"
#include "Platform.h"

#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std::chrono;

namespace PR {
namespace Profiler {
struct CounterEntry {
	Profiler::InternalCounter* CounterPtr;
	const EntryDescription* Desc;

	inline CounterEntry(Profiler::InternalCounter* counter, const EntryDescription* desc)
		: CounterPtr(counter)
		, Desc(desc)
	{
	}
};

struct TimeCounterEntry {
	Profiler::InternalCounter* TotalCounterPtr;
	Profiler::InternalCounter* TimeSpentCounterPtr;
	const EntryDescription* Desc;

	inline TimeCounterEntry(Profiler::InternalCounter* totalCounter,
							Profiler::InternalCounter* timeSpentCounter,
							const EntryDescription* desc)
		: TotalCounterPtr(totalCounter)
		, TimeSpentCounterPtr(timeSpentCounter)
		, Desc(desc)
	{
	}
};

struct ThreadData {
	uint32 ID;
	std::string Name;
	std::vector<CounterEntry> CounterEntries;
	std::vector<TimeCounterEntry> TimeCounterEntries;
	std::vector<std::shared_ptr<Profiler::InternalCounter>> InternalCounters;

	inline explicit ThreadData(uint32 id, const std::string& name)
		: ID(id)
		, Name(name)
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
		sThreadData.emplace_back(id, stream.str());

		return &sThreadData[id];
	} else {
		return &sThreadData[id];
	}
}

// Save to make sure that dll unloads don't affect the profiler
static std::unordered_map<const EntryDescription*, EntryDescription> sSavedEntries;
void saveDescription(const ThreadData* data, const EntryDescription* desc)
{
	if (sSavedEntries.count(desc) == 0) {
		EntryDescription copy = *desc;
		copy.ThreadID		  = data->ID;
		sSavedEntries.insert(std::make_pair(desc, copy));
	}
}

void setThreadName(const std::string& name)
{
	std::lock_guard<std::mutex> guard(sThreadMutex);
	getCurrentThreadData()->Name = name;
}

InternalCounter* registerCounter(const EntryDescription* desc)
{
	PR_ASSERT(desc, "Valid description expected");

	std::lock_guard<std::mutex> guard(sThreadMutex);
	ThreadData* data = getCurrentThreadData();
	saveDescription(data, desc);

	auto counter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(counter);

	data->CounterEntries.emplace_back(counter.get(), desc);
	return counter.get();
}

InternalTimeCounter registerTimeCounter(const EntryDescription* desc)
{
	PR_ASSERT(desc, "Valid description expected");

	std::lock_guard<std::mutex> guard(sThreadMutex);
	ThreadData* data = getCurrentThreadData();
	saveDescription(data, desc);

	auto totalCounter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(totalCounter);
	auto timeSpentCounter = std::make_shared<InternalCounter>(0);
	data->InternalCounters.emplace_back(timeSpentCounter);

	data->TimeCounterEntries.emplace_back(totalCounter.get(), timeSpentCounter.get(), desc);

	return InternalTimeCounter{ totalCounter.get(), timeSpentCounter.get() };
}

struct ProfileCounterSample {
	const EntryDescription* Desc;
	uint64 Value;
};

struct ProfileTimeCounterSample {
	const EntryDescription* Desc;
	uint64 ValueTotal;
	uint64 ValueDuration;
};

struct ProfileSamplePage {
	high_resolution_clock::time_point TimePoint;
	std::vector<ProfileCounterSample> Counters;
	std::vector<ProfileTimeCounterSample> TimeCounters;
};

static std::vector<std::unique_ptr<ProfileSamplePage>> sProfilePages;
static std::atomic<bool> sProfileRun(false);
static std::mutex sProfileMutex;
static high_resolution_clock::time_point sProfileStartTime;
static void profileThread(uint32 samplesPerSecond)
{
	sProfileRun		  = true;
	sProfileStartTime = high_resolution_clock::now();

	ProfileSamplePage* lastPage = nullptr;
	const auto ms				= milliseconds(1000 / samplesPerSecond);
	auto start					= high_resolution_clock::now();
	while (sProfileRun) {
		auto now  = high_resolution_clock::now();
		auto diff = now - start;

		if (duration_cast<milliseconds>(diff) < ms) {
			std::this_thread::sleep_for(ms);
			continue;
		}

		start = now;

		auto page		= std::make_unique<ProfileSamplePage>();
		page->TimePoint = now;
		if (lastPage) {
			page->Counters.reserve(lastPage->Counters.size());
			page->TimeCounters.reserve(lastPage->TimeCounters.size());
		}
		lastPage = page.get();

		sThreadMutex.lock();
		size_t threadID = 0;
		for (const ThreadData& data : sThreadData) {
			for (const CounterEntry& entry : data.CounterEntries) {
				ProfileCounterSample sample;
				sample.Value = *entry.CounterPtr;
				sample.Desc  = entry.Desc;
				page->Counters.push_back(sample);
			}
			for (const TimeCounterEntry& entry : data.TimeCounterEntries) {
				ProfileTimeCounterSample sample;
				sample.ValueTotal	= *entry.TotalCounterPtr;
				sample.ValueDuration = *entry.TimeSpentCounterPtr;
				sample.Desc			 = entry.Desc;
				page->TimeCounters.push_back(sample);
			}
			++threadID;
		}
		sThreadMutex.unlock();

		sProfileMutex.lock();
		sProfilePages.emplace_back(std::move(page));
		sProfileMutex.unlock();
	}
}

static std::unique_ptr<std::thread> sProfileThread;
void start(uint32 samplesPerSecond, int32 /*networkPort*/)
{
	setThreadName("Main");
	sProfileThread = std::make_unique<std::thread>(profileThread, samplesPerSecond);
}

void stop()
{
	sProfileRun = false;
	if (sProfileThread->joinable())
		sProfileThread->join();
	sProfileThread.reset();
}

static void writeStr(std::ofstream& stream, const std::string& str)
{
	uint32 strLen = static_cast<uint32>(str.size());
	stream.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
	if (strLen > 0)
		stream.write(str.c_str(), sizeof(char) * strLen);
}

static void writeDesc(std::ofstream& stream, const EntryDescription& desc)
{
	writeStr(stream, desc.Name);
	writeStr(stream, desc.Function);
	writeStr(stream, desc.File);
	stream.write(reinterpret_cast<const char*>(&desc.Line), sizeof(desc.Line));
	stream.write(reinterpret_cast<const char*>(&desc.ThreadID), sizeof(desc.ThreadID));
	writeStr(stream, desc.Category);
}

bool dumpToFile(const std::wstring& filename)
{
	std::lock_guard<std::mutex> guard(sThreadMutex);

	std::unordered_map<const EntryDescription*, size_t> id_map;

	// Determine amount of unique events and fill the pointer - id map
	uint64 events = 0;
	for (const ThreadData& data : sThreadData) {
		for (const CounterEntry& entry : data.CounterEntries) {
			if (id_map.count(entry.Desc) == 0) {
				auto id			   = id_map.size();
				id_map[entry.Desc] = id;
				++events;
			}
		}
		for (const TimeCounterEntry& entry : data.TimeCounterEntries) {
			if (id_map.count(entry.Desc) == 0) {
				auto id			   = id_map.size();
				id_map[entry.Desc] = id;
				++events;
			}
		}
	}

	std::ofstream stream(encodePath(filename), std::ios::out | std::ios::binary);
	if (!stream)
		return false;

	// Write header
	static const char* HEADER = "PR_PROFILE";
	stream.write(HEADER, 10);

	// Write thread information
	uint64 threads = static_cast<uint64>(sThreadData.size());
	stream.write(reinterpret_cast<const char*>(&threads), sizeof(threads));
	for (const ThreadData& data : sThreadData) {
		writeStr(stream, data.Name);
	}

	// Write events
	stream.write(reinterpret_cast<const char*>(&events), sizeof(events));

	for (const ThreadData& data : sThreadData) {
		for (const CounterEntry& entry : data.CounterEntries) {
			PR_ASSERT(id_map.count(entry.Desc) > 0, "Unexpected entry addition");
			PR_ASSERT(sSavedEntries.count(entry.Desc) > 0, "Invalid entry save");
			writeDesc(stream, sSavedEntries.at(entry.Desc));
		}
		for (const TimeCounterEntry& entry : data.TimeCounterEntries) {
			PR_ASSERT(id_map.count(entry.Desc) > 0, "Unexpected entry addition");
			PR_ASSERT(sSavedEntries.count(entry.Desc) > 0, "Invalid entry save");
			writeDesc(stream, sSavedEntries.at(entry.Desc));
		}
	}

	// Write pages
	sProfileMutex.lock();
	uint64 pages = (uint64)sProfilePages.size();
	stream.write(reinterpret_cast<const char*>(&pages), sizeof(pages));
	for (const auto& page : sProfilePages) {
		uint64 timePoint = static_cast<uint64>(duration_cast<microseconds>(page->TimePoint - sProfileStartTime).count());
		stream.write(reinterpret_cast<const char*>(&timePoint), sizeof(timePoint));

		uint64 counterCount = (uint64)page->Counters.size();
		stream.write(reinterpret_cast<const char*>(&counterCount), sizeof(counterCount));
		for (const auto& counter : page->Counters) {
			PR_ASSERT(id_map.count(counter.Desc) > 0, "Description has to be available");

			uint64 descID = (uint64)id_map[counter.Desc];
			stream.write(reinterpret_cast<const char*>(&descID), sizeof(descID));
			stream.write(reinterpret_cast<const char*>(&counter.Value), sizeof(counter.Value));
		}

		uint64 timeCounterCount = (uint64)page->TimeCounters.size();
		stream.write(reinterpret_cast<const char*>(&timeCounterCount),
					 sizeof(timeCounterCount));
		for (const auto& timeCounter : page->TimeCounters) {
			PR_ASSERT(id_map.count(timeCounter.Desc) > 0, "Description has to be available");

			uint64 descID = (uint64)id_map[timeCounter.Desc];
			stream.write(reinterpret_cast<const char*>(&descID), sizeof(descID));
			stream.write(reinterpret_cast<const char*>(&timeCounter.ValueTotal),
						 sizeof(timeCounter.ValueTotal));
			stream.write(reinterpret_cast<const char*>(&timeCounter.ValueDuration),
						 sizeof(timeCounter.ValueDuration));
		}
	}
	sProfileMutex.unlock();

	return true;
}
} // namespace Profiler
} // namespace PR
