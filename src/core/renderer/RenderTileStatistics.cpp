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
	, mCameraDepthCount(0)
	, mLightDepthCount(0)
{
}

RenderTileStatistics::RenderTileStatistics(const RenderTileStatistics& other)
	: mCameraRayCount(other.mCameraRayCount.load())
	, mLightRayCount(other.mLightRayCount.load())
	, mBounceRayCount(other.mBounceRayCount.load())
	, mShadowRayCount(other.mShadowRayCount.load())
	, mPixelSampleCount(other.mPixelSampleCount.load())
	, mEntityHitCount(other.mEntityHitCount.load())
	, mBackgroundHitCount(other.mBackgroundHitCount.load())
	, mCameraDepthCount(other.mCameraDepthCount.load())
	, mLightDepthCount(other.mLightDepthCount.load())
{
}

RenderTileStatistics& RenderTileStatistics::operator=(const RenderTileStatistics& other)
{
	mCameraRayCount		= other.mCameraRayCount.load();
	mLightRayCount		= other.mLightRayCount.load();
	mBounceRayCount		= other.mBounceRayCount.load();
	mShadowRayCount		= other.mShadowRayCount.load();
	mPixelSampleCount	= other.mPixelSampleCount.load();
	mEntityHitCount		= other.mEntityHitCount.load();
	mBackgroundHitCount = other.mBackgroundHitCount.load();
	mCameraDepthCount	= other.mCameraDepthCount.load();
	mLightDepthCount	= other.mLightDepthCount.load();
	return *this;
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
	mCameraDepthCount += other.mCameraDepthCount;
	mLightDepthCount += other.mLightDepthCount;

	return *this;
}

RenderTileStatistics RenderTileStatistics::half() const
{
	RenderTileStatistics other = *this;
	other.mCameraRayCount	   = other.mCameraRayCount.load() / 2;
	other.mLightRayCount	   = other.mLightRayCount.load() / 2;
	other.mBounceRayCount	   = other.mBounceRayCount.load() / 2;
	other.mShadowRayCount	   = other.mShadowRayCount.load() / 2;
	other.mPixelSampleCount	   = other.mPixelSampleCount.load() / 2;
	other.mEntityHitCount	   = other.mEntityHitCount.load() / 2;
	other.mBackgroundHitCount  = other.mBackgroundHitCount.load() / 2;
	other.mCameraDepthCount	   = other.mCameraDepthCount.load() / 2;
	other.mCameraDepthCount	   = other.mLightDepthCount.load() / 2;

	return other;
}
} // namespace PR
