#include "RenderSettings.h"

namespace PR
{
	RenderSettings::RenderSettings() :
		mDebugMode(DM_None),
		mSamplerMode(SM_Jitter),
		mXSamplerCount(8), mYSamplerCount(8),

		mMaxRayDepth(10),

		// Direct Lightning
		mMaxLightSamples(1),
		mUseBiDirect(true),


		// Photon Mapping
		mMaxPhotons(100000),
		mMaxPhotonGatherRadius(0.02f),
		mMaxPhotonGatherCount(500),
		mMaxPhotonDiffuseBounces(4)
	{
	}
}