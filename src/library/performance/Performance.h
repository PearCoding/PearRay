#pragma once

#include "Logger.h"
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_set>

namespace PR {
class PerformanceEntry {
public:
	typedef std::chrono::high_resolution_clock Clock;
	typedef Clock::time_point TimePoint;

	inline PerformanceEntry(size_t hash, PerformanceEntry* parent,
							int line, const char* file, const char* function);

	inline size_t hash() const;
	inline PerformanceEntry* parent() const;
	inline int line() const;
	inline const char* file() const;
	inline const char* function() const;

	inline uint64 ticks() const;

	inline void start();
	inline void end();

	inline std::list<PerformanceEntry*>& children();
	inline const std::list<PerformanceEntry*>& children() const;

private:
	size_t mHash;
	PerformanceEntry* mParent;
	std::list<PerformanceEntry*> mChildren;
	int mLine;
	const char* mFile;
	const char* mFunction;
	TimePoint mStart;

	uint64 mTicks; // Micro
};

/* Only one instance per application! */
class PR_LIB PerformanceManager {
	friend class PerformanceWriter;

public:
	class SetEntry {
	public:
		PerformanceEntry* Entry;
		size_t Hash;

		inline explicit SetEntry(PerformanceEntry* e)
			: Entry(e)
			, Hash(e->hash())
		{
		}

		inline explicit SetEntry(size_t hash)
			: Entry(nullptr)
			, Hash(hash)
		{
		}

		inline bool operator==(const SetEntry& o) const
		{
			return Hash == o.Hash;
		}
	};
	typedef size_t (*hasher)(const SetEntry&);
	typedef std::unordered_set<SetEntry, hasher> Set;

	inline PerformanceManager();
	inline ~PerformanceManager();

	inline PerformanceEntry* add(int line, const char* file, const char* function);
	inline void end(PerformanceEntry* e);

	static inline PerformanceManager& instance()
	{
		static PerformanceManager perf;
		return perf;
	}

	inline const std::map<std::thread::id, Set>& entries() const;

private:
	std::map<std::thread::id, Set> mEntries;
	static thread_local PerformanceEntry* mCurrentParent;
	std::mutex mMutex;
};

class PerformanceGuard {
public:
	explicit PerformanceGuard(PerformanceEntry* e)
		: mEntry(e)
	{
	}

	~PerformanceGuard()
	{
		PerformanceManager::instance().end(mEntry);
	}

private:
	PerformanceEntry* mEntry;
};
}

#ifdef PR_PROFILE
#define PR_GUARD_PROFILE_ID(id) \
	PR::PerformanceGuard _perf_guard_##id(PR::PerformanceManager::instance().add(__LINE__, __FILE__, PR_FUNCTION_NAME))
#define PR_BEGIN_PROFILE_ID(id) \
	PR::PerformanceEntry* _perf_e_##id = PR::PerformanceManager::instance().add(__LINE__, __FILE__, PR_FUNCTION_NAME)
#define PR_END_PROFILE_ID(id) \
	PR::PerformanceManager::instance().end(_perf_e_##id)
#else
#define PR_GUARD_PROFILE_ID(id)
#define PR_BEGIN_PROFILE_ID(id)
#define PR_END_PROFILE_ID(id)
#endif

#define PR_GUARD_PROFILE() PR_GUARD_PROFILE_ID(0)
#define PR_BEGIN_PROFILE() PR_BEGIN_PROFILE_ID(0)
#define PR_END_PROFILE() PR_END_PROFILE_ID(0)

#include "Performance.inl"
