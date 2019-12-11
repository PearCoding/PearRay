#include "RenderSettings.h"
#include "filter/FilterFactory.h"
#include "sampler/SamplerFactory.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxRayDepth(8)
	, maxParallelRays(10000)
	, aaSampleCount(1)
	, lensSampleCount(1)
	, timeSampleCount(1)
	, aaSampler("multi_jitter")
	, lensSampler("multi_jitter")
	, timeSampler("multi_jitter")
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
	, pixelFilter("gaussian")
{
}

std::shared_ptr<ISampler> RenderSettings::createAASampler(Random& random) const
{
	return SamplerFactory::createSampler(aaSampler, random, aaSampleCount);
}

std::shared_ptr<ISampler> RenderSettings::createLensSampler(Random& random) const
{
	return SamplerFactory::createSampler(lensSampler, random, lensSampleCount);
}

std::shared_ptr<ISampler> RenderSettings::createTimeSampler(Random& random) const
{
	return SamplerFactory::createSampler(timeSampler, random, timeSampleCount);
}

std::shared_ptr<IFilter> RenderSettings::createPixelFilter() const
{
	return FilterFactory::createFilter(pixelFilter, pixelFilterRadius);
}

} // namespace PR
