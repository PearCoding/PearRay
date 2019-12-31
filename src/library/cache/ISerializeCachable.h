#pragma once

#include "ICachable.h"
#include "serialization/ISerializable.h"

#include <atomic>
#include <tbb/queuing_rw_mutex.h>

namespace PR {
class Cache;
class PR_LIB ISerializeCachable : public ICachable, public ISerializable {
public:
	ISerializeCachable(const std::string& name, const std::shared_ptr<Cache>& cache, bool useCache);
	virtual ~ISerializeCachable();

	inline bool isLoaded() const override { return mIsLoaded; }
	inline size_t memoryUsage() const override { return mMemoryUsage; }
	inline size_t accessCount() const override { return mAccessCount; }

	inline std::string name() const { return mName; }
	inline std::wstring cacheFileNoExt() const { return mCacheFileNoExt; }
	inline std::wstring cacheFile() const { return cacheFileNoExt() + L".bin"; }
	inline std::shared_ptr<Cache> cache() const { return mCache; }

protected:
	using Mutex = tbb::queuing_rw_mutex;
	using Lock  = Mutex::scoped_lock;

	template <typename Func>
	inline auto doWork(Func func)
	{
		++mAccessCount;

		if (mUseCache) {
			Lock lock(mWorkMutex, false);
			if (!mIsLoaded)
				handleLoadFromCache(lock);

			return func();
		} else {
			return func();
		}
	}

	void load() override;
	void unload() override;

	virtual void beforeLoad() {}
	virtual void afterLoad() {}
	virtual void beforeUnload() {}
	virtual void afterUnload() {}

private:
	void handleLoadFromCache(Lock& lock);
	void handle(bool read);

	const std::string mName;
	const std::wstring mCacheFileNoExt;
	size_t mMemoryUsage;

	std::atomic<bool> mIsLoaded;
	std::atomic<size_t> mAccessCount;

	Mutex mWorkMutex;

	std::shared_ptr<Cache> mCache;
	bool mUseCache;
};
} // namespace PR