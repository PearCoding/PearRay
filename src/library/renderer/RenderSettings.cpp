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

		// Crop
		mCropMaxX(1), mCropMinX(0),
		mCropMaxY(1), mCropMinY(0),

		// Direct Lightning
		mMaxLightSamples(1),

		// Photon Mapping
		mMaxPhotons(100000),
		mMaxPhotonGatherRadius(0.1f),
		mMaxPhotonGatherCount(500),
		mMaxPhotonDiffuseBounces(4),
		mMinPhotonSpecularBounces(1),
		mPhotonGatheringMode(PGM_Sphere),
		mPhotonSqueezeWeight(0)
	{
	}
}