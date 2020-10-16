#include "Cache.h"

#include <filesystem>

namespace PR {
Cache::Cache(const std::filesystem::path& workDir)
	: mCacheDir(workDir / "cache/")
	, mMaxMemoryUsage(4ULL * 1024 * 1024 * 1024) // 4 GB
	, mMode(CM_Auto)
	, mCurrentMemoryUsage(0)
{
	std::filesystem::create_directories(mCacheDir);
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

bool Cache::shouldCacheMesh(size_t nodecount, CacheMode local) const
{
	constexpr size_t MID = 500000;
	switch (mMode) {
	case CM_None:
		return false;
	case CM_All:
		return true;
	default:
	case CM_Auto:
		switch (local) {
		case CM_None:
			return false;
		case CM_All:
			return true;
		default:
		case CM_Auto:
			return nodecount >= MID;
		}
	}
}
} // namespace PR