#include "RenderSettings.h"
#include "filter/IFilterFactory.h"
#include "integrator/IIntegratorFactory.h"
#include "sampler/ISamplerFactory.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxParallelRays(10000)
	, timeMappingMode(TMM_RIGHT)
	, timeScale(1)
	, tileMode(TM_LINEAR)
	, useAdaptiveTiling(true)
	, sortHits(false)
	, spectralStart(360)
	, spectralEnd(760)
	, filmWidth(1920)
	, filmHeight(1080)
	, cropMinX(0)
	, cropMaxX(1)
	, cropMinY(0)
	, cropMaxY(1)
{
}

RenderSettings::~RenderSettings()
{
}

std::shared_ptr<ISampler> RenderSettings::createAASampler(Random& random) const
{
	return aaSamplerFactory ? aaSamplerFactory->createInstance(random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createLensSampler(Random& random) const
{
	return lensSamplerFactory ? lensSamplerFactory->createInstance(random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createTimeSampler(Random& random) const
{
	return timeSamplerFactory ? timeSamplerFactory->createInstance(random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createSpectralSampler(Random& random) const
{
	return spectralSamplerFactory ? spectralSamplerFactory->createInstance(random) : nullptr;
}

std::shared_ptr<IFilter> RenderSettings::createPixelFilter() const
{
	return pixelFilterFactory ? pixelFilterFactory->createInstance() : nullptr;
}

std::shared_ptr<IIntegrator> RenderSettings::createIntegrator() const
{
	return integratorFactory ? integratorFactory->createInstance() : nullptr;
}
} // namespace PR
