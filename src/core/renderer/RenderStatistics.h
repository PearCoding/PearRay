#pragma once

#include "PR_Config.h"

#include <atomic>

namespace PR {
enum RenderStatisticTypes {
	RST_CameraRayCount = 0,
	RST_LightRayCount,
	RST_PrimaryRayCount,
	RST_BounceRayCount,
	RST_ShadowRayCount,
	RST_MonochromeRayCount,
	RST_PixelSampleCount,
	RST_EntityHitCount,
	RST_BackgroundHitCount,
	RST_CameraDepthCount,
	RST_LightDepthCount,

	_RST_COUNT_
};

class PR_LIB_CORE RenderStatistics {
public:
	RenderStatistics();
	RenderStatistics(const RenderStatistics& other);

	RenderStatistics& operator=(const RenderStatistics& other);
	RenderStatistics& operator+=(const RenderStatistics& other);

	inline uint64 rayCount() const { return entry(RST_PrimaryRayCount) + entry(RST_BounceRayCount) + entry(RST_ShadowRayCount); }
	inline uint64 depthCount() const { return entry(RST_CameraDepthCount) + entry(RST_LightDepthCount); }

	inline void add(RenderStatisticTypes rst, uint64 i = 1) { mCounters[rst] += i; }
	inline uint64 entry(RenderStatisticTypes rst) const { return mCounters[rst]; }

	RenderStatistics half() const;

private:
	std::array<std::atomic<uint64>, _RST_COUNT_> mCounters;
};
} // namespace PR
