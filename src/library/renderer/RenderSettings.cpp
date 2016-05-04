#include "RenderSettings.h"

namespace PR
{
	RenderSettings::RenderSettings() :
		mSamplerMode(SM_Jitter),
		mXSamplerCount(8), mYSamplerCount(8),

		mMaxRayDepth(10),

		// Direct Lightning
		mMaxLightSamples(1),

		// Photon Mapping
		mMaxPhotons(100000),
		mMaxPhotonGatherRadius(0.02f),
		mMaxPhotonGatherCount(500)
	{
	}
}