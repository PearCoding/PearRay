#include "RenderTileStatistics.h"

namespace PR {
RenderTileStatistics::RenderTileStatistics()
	: mCameraRayCount(0)
	, mLightRayCount(0)
	, mBounceRayCount(0)
	, mShadowRayCount(0)
	, mPixelSampleCount(0)
	, mEntityHitCount(0)
	, mBackgroundHitCount(0)
{
}

RenderTileStatistics& RenderTileStatistics::operator+=(const RenderTileStatistics& other)
{
	mCameraRayCount += other.mCameraRayCount;
	mLightRayCount += other.mLightRayCount;
	mBounceRayCount += other.mBounceRayCount;
	mShadowRayCount += other.mShadowRayCount;
	mPixelSampleCount += other.mPixelSampleCount;
	mEntityHitCount += other.mEntityHitCount;
	mBackgroundHitCount += other.mBackgroundHitCount;

	return *this;
}
} // namespace PR
