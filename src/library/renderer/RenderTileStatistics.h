#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB RenderTileStatistics {
public:
	RenderTileStatistics();

	RenderTileStatistics& operator+=(const RenderTileStatistics& other);

	inline void addCoherentRayCount(uint64 i = 1) { mCoherentRayCount += i; }
	inline uint64 coherentRayCount() const { return mCoherentRayCount; }

	inline void addIncoherentRayCount(uint64 i = 1) { mIncoherentRayCount += i; }
	inline uint64 incoherentRayCount() const { return mIncoherentRayCount; }

	inline void addShadowRayCount(uint32 i = 1) { mShadowRayCount += i; }
	inline uint64 shadowRayCount() const { return mShadowRayCount; }

	inline uint64 rayCount() const { return coherentRayCount() + incoherentRayCount() + shadowRayCount(); }

	inline void addPixelSampleCount(uint64 i = 1) { mPixelSampleCount += i; }
	inline uint64 pixelSampleCount() const { return mPixelSampleCount; }

	inline void addEntityHitCount(uint64 i = 1) { mEntityHitCount += i; }
	inline uint64 entityHitCount() const { return mEntityHitCount; }

	inline void addBackgroundHitCount(uint64 i = 1) { mBackgroundHitCount += i; }
	inline uint64 backgroundHitCount() const { return mBackgroundHitCount; }

private:
	uint64 mCoherentRayCount;
	uint64 mIncoherentRayCount;
	uint64 mShadowRayCount;
	uint64 mPixelSampleCount;
	uint64 mEntityHitCount;
	uint64 mBackgroundHitCount;
};
} // namespace PR
