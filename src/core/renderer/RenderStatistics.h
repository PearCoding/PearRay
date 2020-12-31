#pragma once

#include "PR_Config.h"

#include <array>
#include <atomic>

namespace PR {
enum class RenderStatisticEntry {
	CameraRayCount = 0,
	LightRayCount,
	PrimaryRayCount,
	BounceRayCount,
	ShadowRayCount,
	MonochromeRayCount,
	PixelSampleCount,
	EntityHitCount,
	BackgroundHitCount,
	CameraDepthCount,
	LightDepthCount,

	_COUNT
};

class PR_LIB_CORE RenderStatistics {
public:
	RenderStatistics();
	RenderStatistics(const RenderStatistics& other);

	RenderStatistics& operator=(const RenderStatistics& other);
	RenderStatistics& operator+=(const RenderStatistics& other);

	inline uint64 rayCount() const
	{
		return entry(RenderStatisticEntry::PrimaryRayCount) + entry(RenderStatisticEntry::BounceRayCount) + entry(RenderStatisticEntry::ShadowRayCount);
	}
	inline uint64 depthCount() const
	{
		return entry(RenderStatisticEntry::CameraDepthCount) + entry(RenderStatisticEntry::LightDepthCount);
	}

	inline void add(RenderStatisticEntry rse, uint64 i = 1) { mCounters[(uint32)rse] += i; }
	inline uint64 entry(RenderStatisticEntry rse) const { return mCounters[(uint32)rse]; }

	RenderStatistics half() const;

private:
	std::array<std::atomic<uint64>, (uint32)RenderStatisticEntry::_COUNT> mCounters;
};
} // namespace PR
