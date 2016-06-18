#include "RenderSettings.h"

namespace PR
{
	RenderSettings::RenderSettings() :
		mIncremental(true),
		mDebugMode(DM_None),
		mPixelSampler(SM_MultiJitter),
		mMaxPixelSampleCount(64),

		mMaxDiffuseBounces(2),
		mMaxRayDepth(10),

		// Direct Lightning
		mMaxLightSamples(1),
		mUseBiDirect(true),


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