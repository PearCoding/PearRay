#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_CORE RenderTileStatistics {
public:
	RenderTileStatistics();

	RenderTileStatistics& operator+=(const RenderTileStatistics& other);

	inline uint64 rayCount() const { return cameraRayCount() + lightRayCount() + bounceRayCount() + shadowRayCount(); }

	inline void addCameraRayCount(uint64 i = 1) { mCameraRayCount += i; }
	inline uint64 cameraRayCount() const { return mCameraRayCount; }

	inline void addLightRayCount(uint64 i = 1) { mLightRayCount += i; }
	inline uint64 lightRayCount() const { return mLightRayCount; }

	inline void addBounceRayCount(uint64 i = 1) { mBounceRayCount += i; }
	inline uint64 bounceRayCount() const { return mBounceRayCount; }

	inline void addShadowRayCount(uint32 i = 1) { mShadowRayCount += i; }
	inline uint64 shadowRayCount() const { return mShadowRayCount; }

	inline void addPixelSampleCount(uint64 i = 1) { mPixelSampleCount += i; }
	inline uint64 pixelSampleCount() const { return mPixelSampleCount; }

	inline void addEntityHitCount(uint64 i = 1) { mEntityHitCount += i; }
	inline uint64 entityHitCount() const { return mEntityHitCount; }

	inline void addBackgroundHitCount(uint64 i = 1) { mBackgroundHitCount += i; }
	inline uint64 backgroundHitCount() const { return mBackgroundHitCount; }

	inline void addDepthCount(uint64 i = 1) { mDepthCount += i; }
	inline uint64 depthCount() const { return mDepthCount; }

	RenderTileStatistics half() const;

private:
	uint64 mCameraRayCount;
	uint64 mLightRayCount;
	uint64 mBounceRayCount;
	uint64 mShadowRayCount;
	uint64 mPixelSampleCount;
	uint64 mEntityHitCount;
	uint64 mBackgroundHitCount;
	uint64 mDepthCount;
};
} // namespace PR
