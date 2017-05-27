#include "PPMSettings.h"

namespace PR {
PPMSettings::PPMSettings()
	: mMaxPhotonsPerPass(100000)
	, mMaxPassCount(50)
	, mMaxGatherRadius(0.1f)
	, mMaxGatherCount(500)
	, mGatheringMode(PGM_Sphere)
	, mSqueezeWeight(0)
	, mContractRatio(0.3f)
	, mProjectionMapWeight(0.0f) // Disable for now
	, mProjectionMapQuality(0.7f)
	, mProjectionMapPreferCaustic(0.2f)
{
}
}
