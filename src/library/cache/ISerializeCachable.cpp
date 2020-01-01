#include "ISerializeCachable.h"
#include "Cache.h"
#include "Logger.h"
#include "serialization/FileSerializer.h"

#include <boost/filesystem.hpp>

namespace PR {
ISerializeCachable::ISerializeCachable(const std::string& name,
									   const std::shared_ptr<Cache>& cache,
									   bool useCache)
	: mName(name)
	, mCacheFileNoExt((boost::filesystem::path(cache->cacheDir()) / mName).generic_wstring())
	, mMemoryUsage(0)
	, mIsLoaded(true)
	, mAccessCount(0)
	, mCache(cache)
	, mUseCache(useCache)
{
	if (mUseCache) {
		PR_LOG(L_DEBUG) << name << " is cached" << std::endl;
		cache->add(this);
	}
}

ISerializeCachable::~ISerializeCachable()
{
}

void ISerializeCachable::handleLoadFromCache(Lock& lock)
{
	lock.upgrade_to_writer();
	mCache->load(this);
	lock.downgrade_to_reader();
}

void ISerializeCachable::load()
{
	if (mIsLoaded)
		return;

	beforeLoad();
	handle(true);
	mIsLoaded = true;
	afterLoad();
}

void ISerializeCachable::unload()
{
	if (!mIsLoaded)
		return;

	beforeUnload();
	handle(false);
	mIsLoaded = false;
	afterUnload();
}

void ISerializeCachable::handle(bool read)
{
	FileSerializer serializer;

	if (!serializer.open(cacheFile(), read))
		return;

	this->serialize(serializer);
	mMemoryUsage = serializer.memoryFootprint();
}
} // namespace PR