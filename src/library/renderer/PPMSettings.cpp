#include "PPMSettings.h"

namespace PR
{
	PPMSettings::PPMSettings() :
		mMaxPhotonsPerPass(100000),
		mMaxPassCount(50),
		mMaxGatherRadius(0.1f),
		mMaxGatherCount(500),
		mGatheringMode(PGM_Sphere),
		mSqueezeWeight(0)
	{
	}
}