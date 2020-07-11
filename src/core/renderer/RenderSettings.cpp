#include "RenderSettings.h"
#include "filter/IFilterFactory.h"
#include "integrator/IIntegratorFactory.h"
#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxParallelRays(10000)
	, timeMappingMode(TMM_RIGHT)
	, timeScale(1)
	, tileMode(TM_ZORDER)
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
	return aaSamplerFactory ? aaSamplerFactory->createInstance(maxSampleCount(), random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createLensSampler(Random& random) const
{
	return lensSamplerFactory ? lensSamplerFactory->createInstance(maxSampleCount(), random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createTimeSampler(Random& random) const
{
	return timeSamplerFactory ? timeSamplerFactory->createInstance(maxSampleCount(), random) : nullptr;
}

std::shared_ptr<ISampler> RenderSettings::createSpectralSampler(Random& random) const
{
	return spectralSamplerFactory ? spectralSamplerFactory->createInstance(maxSampleCount(), random) : nullptr;
}

std::shared_ptr<IFilter> RenderSettings::createPixelFilter() const
{
	return pixelFilterFactory ? pixelFilterFactory->createInstance() : nullptr;
}

std::shared_ptr<IIntegrator> RenderSettings::createIntegrator() const
{
	return integratorFactory ? integratorFactory->createInstance() : nullptr;
}

uint32 RenderSettings::maxSampleCount() const
{
	PR_ASSERT(aaSamplerFactory && lensSamplerFactory && timeSamplerFactory && spectralSamplerFactory, "Expect all samplers to be constructed");
	return aaSamplerFactory->requestedSampleCount()
		   * lensSamplerFactory->requestedSampleCount()
		   * timeSamplerFactory->requestedSampleCount()
		   * spectralSamplerFactory->requestedSampleCount();
}
} // namespace PR
