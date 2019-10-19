#include "RenderSettings.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxRayDepth(8)
	, aaSampleCount(1)
	, lensSampleCount(1)
	, timeSampleCount(1)
	, spectralSampleCount(1)
	, aaSampler(SM_MULTI_JITTER)
	, lensSampler(SM_MULTI_JITTER)
	, timeSampler(SM_MULTI_JITTER)
	, spectralSampler(SM_MULTI_JITTER)
	, spectralProcessMode(SPM_LINEAR)
	, timeMappingMode(TMM_RIGHT)
	, timeScale(1)
	, tileMode(TM_LINEAR)
	, filmWidth(1920)
	, filmHeight(1080)
	, cropMinX(0)
	, cropMaxX(1)
	, cropMinY(0)
	, cropMaxY(1)
{
}

} // namespace PR
