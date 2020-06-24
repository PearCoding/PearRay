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
	, mDepthCount(0)
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
	mDepthCount += other.mDepthCount;

	return *this;
}

RenderTileStatistics RenderTileStatistics::half() const
{
	RenderTileStatistics other = *this;
	other.mCameraRayCount /= 2;
	other.mLightRayCount /= 2;
	other.mBounceRayCount /= 2;
	other.mShadowRayCount /= 2;
	other.mPixelSampleCount /= 2;
	other.mEntityHitCount /= 2;
	other.mBackgroundHitCount /= 2;
	other.mDepthCount /= 2;

	return other;
}
} // namespace PR
