#pragma once

#include "Config.h"

namespace PR
{
	class PR_LIB RenderStatistics
	{
	public:
		RenderStatistics();

		RenderStatistics& operator +=(const RenderStatistics& other);

		inline void incRayCount() { mRayCount++; }
		inline uint64 rayCount() { return mRayCount; }

		inline void incPixelSampleCount() { mPixelSampleCount++; }
		inline uint64 pixelSampleCount() { return mPixelSampleCount; }

		inline void incEntityHitCount() { mEntityHitCount++; }
		inline uint64 entityHitCount() { return mEntityHitCount; }

		inline void incBackgroundHitCount() { mBackgroundHitCount++; }
		inline uint64 backgroundHitCount() { return mBackgroundHitCount; }
	private:
		uint64 mRayCount;
		uint64 mPixelSampleCount;
		uint64 mEntityHitCount;
		uint64 mBackgroundHitCount;
	};
}