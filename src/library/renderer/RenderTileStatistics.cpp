#include "RenderTileStatistics.h"

namespace PR {
RenderTileStatistics::RenderTileStatistics()
	: mCoherentRayCount(0)
	, mIncoherentRayCount(0)
	, mShadowRayCount(0)
	, mPixelSampleCount(0)
	, mEntityHitCount(0)
	, mBackgroundHitCount(0)
{
}

RenderTileStatistics& RenderTileStatistics::operator+=(const RenderTileStatistics& other)
{
	mCoherentRayCount += other.mCoherentRayCount;
	mIncoherentRayCount += other.mIncoherentRayCount;
	mShadowRayCount += other.mShadowRayCount;
	mPixelSampleCount += other.mPixelSampleCount;
	mEntityHitCount += other.mEntityHitCount;
	mBackgroundHitCount += other.mBackgroundHitCount;

	return *this;
}
} // namespace PR
