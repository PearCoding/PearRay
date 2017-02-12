#include "RenderStatistics.h"

namespace PR
{
	RenderStatistics::RenderStatistics() :
		mRayCount(0),
		mPixelSampleCount(0),
		mEntityHitCount(0),
		mBackgroundHitCount(0)
	{
	}

	RenderStatistics& RenderStatistics::operator +=(const RenderStatistics& other)
	{
		mRayCount += other.mRayCount;
		mPixelSampleCount += other.mPixelSampleCount;
		mEntityHitCount += other.mEntityHitCount;
		mBackgroundHitCount += other.mBackgroundHitCount;

		return *this;
	}
}
