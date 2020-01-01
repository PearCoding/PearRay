#pragma once

#include "ICachable.h"
#include <atomic>
#include <mutex>
#include <tbb/concurrent_priority_queue.h>

namespace PR {
enum CacheMode {
	CM_None = 0,
	CM_Auto,
	CM_All
};

class ICachable;
class PR_LIB Cache {
public:
	Cache(const std::wstring& workDir);
	~Cache();

	/**
	 * @brief Add entity to the list of loaded entities, regardless if loaded or not
	 * 
	 * @param entity 
	 */
	void add(ICachable* entity);
	/**
	 * @brief Load entity and add to the list of loaded entities
	 * 
	 * @param entity 
	 */
	void load(ICachable* entity);
	void unloadAll();

	inline const std::wstring& cacheDir() const { return mCacheDir; }
	inline void setMaxMemoryUsage(size_t mem) { mMaxMemoryUsage = mem; }
	inline size_t maxMemoryUsage() const { return mMaxMemoryUsage; }
	inline size_t currentMemoryUsage() const { return mMaxMemoryUsage; }

	inline size_t loadedEntityCount() const { return mEntities.size(); }

	inline CacheMode mode() const { return mMode; }
	inline void setMode(CacheMode mode) { mMode = mode; }

	bool shouldCacheMesh(size_t nodecount, CacheMode local = CM_Auto) const;

private:
	struct Comparator {
		bool operator()(const ICachable* lhs, const ICachable* rhs) const
		{
			return lhs->accessCount() < rhs->accessCount();
		}
	};
	tbb::concurrent_priority_queue<ICachable*, Comparator> mEntities;

	// This two variable do not change while working -> No thread safety needed
	const std::wstring mCacheDir;
	size_t mMaxMemoryUsage;
	CacheMode mMode;

	std::atomic<size_t> mCurrentMemoryUsage;
};
} // namespace PR