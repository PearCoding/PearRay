#pragma once

#include "RayGroup.h"
#include <vector>

namespace PR {

/// Non thread safe container for ray groups
class PR_LIB_CORE RayGroupContainer {
public:
	inline uint32 registerGroup(const RayGroup& grp)
	{
		PR_ASSERT(mGroups.size() < std::numeric_limits<uint32>::max(), "Way too many groups registered");

		uint32 id = static_cast<uint32>(mGroups.size());
		mGroups.emplace_back(grp);
		return id;
	}

	inline uint32 registerGroup(RayGroup&& grp)
	{
		PR_ASSERT(mGroups.size() < std::numeric_limits<uint32>::max(), "Way too many groups registered");

		uint32 id = static_cast<uint32>(mGroups.size());
		mGroups.emplace_back(std::move(grp));
		return id;
	}

	inline const RayGroup& group(uint32 id) const
	{
		PR_ASSERT(id < mGroups.size(), "Invalid access");
		return mGroups[id];
	}

	inline void reset()
	{
		mGroups.clear();
	}

private:
	std::vector<RayGroup> mGroups;
};
} // namespace PR
