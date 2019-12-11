#include "RenderSettings.h"
#include "filter/FilterFactory.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxRayDepth(8)
	, maxParallelRays(10000)
	, aaSampleCount(1)
	, lensSampleCount(1)
	, timeSampleCount(1)
	, aaSampler(SM_MULTI_JITTER)
	, lensSampler(SM_MULTI_JITTER)
	, timeSampler(SM_MULTI_JITTER)
	, timeMappingMode(TMM_RIGHT)
	, timeScale(1)
	, tileMode(TM_LINEAR)
	, filmWidth(1920)
	, filmHeight(1080)
	, cropMinX(0)
	, cropMaxX(1)
	, cropMinY(0)
	, cropMaxY(1)
	, pixelFilterRadius(1)
	, pixelFilter("triangle")
{
}

std::shared_ptr<IFilter> RenderSettings::createPixelFilter() const
{
	return FilterFactory::createFilter(pixelFilter, pixelFilterRadius);
}

} // namespace PR
