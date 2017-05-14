#include "RenderSettings.h"

namespace PR
{
	RenderSettings::RenderSettings() :
		mSeed((uint64_t)time(NULL)),
		
		mIncremental(true),
		mDebugMode(DM_None),
		mIntegratorMode(IM_BiDirect),

		mAASampler(SM_MultiJitter),
		mMaxAASampleCount(4),
		mLensSampler(SM_MultiJitter),
		mMaxLensSampleCount(1),

		mTimeSampler(SM_MultiJitter),
		mMaxTimeSampleCount(1),
		mTimeMappingMode(TMM_Center),
		mTimeScale(1),

		mSpectralSampler(SM_MultiJitter),
		mMaxSpectralSampleCount(1),

		mMaxDiffuseBounces(2),
		mMaxRayDepth(10),

		// Crop
		mCropMaxX(1), mCropMinX(0),
		mCropMaxY(1), mCropMinY(0),

		// (Bi-)Direct Lightning
		mMaxLightSamples(2),

		// PPM
		mPPM(),

		// Display Rendering
		mTileMode(TM_Linear)
	{
	}
}
