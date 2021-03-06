#include "RenderSettings.h"
#include "filter/IFilterFactory.h"
#include "integrator/IIntegratorFactory.h"
#include "sampler/ISampler.h"
#include "sampler/ISamplerFactory.h"
#include "spectral/CIE.h"
#include "spectral/ISpectralMapper.h"
#include "spectral/ISpectralMapperFactory.h"

namespace PR {
RenderSettings::RenderSettings()
	: seed(42)
	, maxParallelRays(10000)
	, sampleCountOverride(0)
	, timeMappingMode(TimeMappingMode::Right)
	, timeScale(1)
	, tileMode(TileMode::ZOrder)
	, useAdaptiveTiling(true)
	, sortHits(false)
	, progressive(false)
	, spectralStart(PR_CIE_WAVELENGTH_START)
	, spectralEnd(PR_CIE_WAVELENGTH_END)
	, spectralMono(false)
	, spectralHero(true)
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

std::shared_ptr<ISpectralMapper> RenderSettings::createSpectralMapper(const std::string& purpose, RenderContext* ctx) const
{
	if (spectralMapperFactories.count(purpose))
		return spectralMapperFactories.at(purpose)->createInstance(ctx);
	else
		return nullptr;
}

uint32 RenderSettings::maxSampleCount() const
{
	PR_ASSERT(aaSamplerFactory && lensSamplerFactory && timeSamplerFactory && spectralSamplerFactory, "Expect all samplers to be constructed");
	if (progressive)
		return 0;
	else if (sampleCountOverride > 0)
		return sampleCountOverride;
	else
		return aaSamplerFactory->requestedSampleCount()
			   * lensSamplerFactory->requestedSampleCount()
			   * timeSamplerFactory->requestedSampleCount()
			   * spectralSamplerFactory->requestedSampleCount();
}
} // namespace PR
