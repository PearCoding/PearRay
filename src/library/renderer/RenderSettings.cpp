#include "RenderSettings.h"

namespace PR
{
	RenderSettings::RenderSettings() :
		mIncremental(true),
		mDebugMode(DM_None),
		mIntegratorMode(IM_BiDirect),
		mPixelSampler(SM_MultiJitter),
		mMaxPixelSampleCount(64),

		mMaxDiffuseBounces(2),
		mMaxRayDepth(10),

		mUnitScale(0.01f),// For future use

		// Crop
		mCropMaxX(1), mCropMinX(0),
		mCropMaxY(1), mCropMinY(0),

		// Direct Lightning
		mMaxLightSamples(1),

		// PPM
		mPPM()
	{
	}
}