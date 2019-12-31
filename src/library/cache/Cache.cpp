#include "Cache.h"

#include <boost/filesystem.hpp>

namespace PR {
Cache::Cache(const std::wstring& workDir)
	: mCacheDir(workDir + L"/cache/")
	, mMaxMemoryUsage(4ULL * 1024 * 1024 * 1024) // 4 GB
	, mCurrentMemoryUsage(0)
{
	boost::filesystem::create_directories(mCacheDir);
}

Cache::~Cache()
{
}

void Cache::add(ICachable* entity)
{
	size_t usage  = entity->memoryUsage();
	size_t newmem = mCurrentMemoryUsage + usage;
	if (newmem > mMaxMemoryUsage) {
		// Unload till fit
		do {
			ICachable* other;
			if (!mEntities.try_pop(other))
				break;
			PR_ASSERT(other != entity, "Entity was already added!");

			other->unload();
			newmem -= other->memoryUsage();
		} while (newmem > mMaxMemoryUsage);
	}

	mEntities.push(entity);
	mCurrentMemoryUsage += usage;
}

void Cache::load(ICachable* entity)
{
	add(entity);
	if (!entity->isLoaded())
		entity->load();
}

void Cache::unloadAll()
{
	ICachable* entity;
	while (mEntities.try_pop(entity))
		entity->unload();

	mCurrentMemoryUsage = 0;
}

} // namespace PR