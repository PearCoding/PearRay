#include "RenderSettings.h"

namespace PR {
RenderSettings::RenderSettings(const std::shared_ptr<Registry>& registry)
	: mRegistry(registry)
{
}

uint64 RenderSettings::seed() const
{
	return mRegistry->getByGroup<uint64>(RG_RENDERER,
										 "common/seed",
										 42);
}

uint32 RenderSettings::maxRayDepth() const
{
	return std::max<uint32>(1,
							mRegistry->getByGroup<uint32>(RG_RENDERER,
														  "common/max_ray_depth",
														  1));
}

uint64 RenderSettings::aaSampleCount() const
{
	return std::max<uint64>(1,
							mRegistry->getByGroup<uint64>(RG_RENDERER,
														  "common/sampler/aa/count",
														  1));
}

uint64 RenderSettings::lensSampleCount() const
{
	return std::max<uint64>(1,
							mRegistry->getByGroup<uint64>(RG_RENDERER,
														  "common/sampler/lens/count",
														  1));
}

uint64 RenderSettings::timeSampleCount() const
{
	return std::max<uint64>(1,
							mRegistry->getByGroup<uint64>(RG_RENDERER,
														  "common/sampler/time/count",
														  1));
}

uint64 RenderSettings::spectralSampleCount() const
{
	return std::max<uint64>(1,
							mRegistry->getByGroup<uint64>(RG_RENDERER,
														  "common/sampler/spectral/count",
														  1));
}

SamplerMode RenderSettings::aaSampler() const
{
	return mRegistry->getByGroup<SamplerMode>(RG_RENDERER,
											  "common/sampler/aa/type",
											  SM_MULTI_JITTER);
}

SamplerMode RenderSettings::lensSampler() const
{
	return mRegistry->getByGroup<SamplerMode>(RG_RENDERER,
											  "common/sampler/lens/type",
											  SM_MULTI_JITTER);
}

SamplerMode RenderSettings::timeSampler() const
{
	return mRegistry->getByGroup<SamplerMode>(RG_RENDERER,
											  "common/sampler/time/type",
											  SM_MULTI_JITTER);
}

SamplerMode RenderSettings::spectralSampler() const
{
	return mRegistry->getByGroup<SamplerMode>(RG_RENDERER,
											  "common/sampler/spectral/type",
											  SM_MULTI_JITTER);
}

TimeMappingMode RenderSettings::timeMappingMode() const
{
	return mRegistry->getByGroup<TimeMappingMode>(RG_RENDERER,
												  "common/sampler/time/mapping",
												  TMM_RIGHT);
}

float RenderSettings::timeScale() const
{
	return mRegistry->getByGroup<float>(RG_RENDERER,
										"common/sampler/time/scale",
										1.0);
}

IntegratorMode RenderSettings::integratorMode() const
{
	return mRegistry->getByGroup<IntegratorMode>(RG_RENDERER,
												 "common/type",
												 IM_BIDIRECT);
}

TileMode RenderSettings::tileMode() const
{
	return mRegistry->getByGroup<TileMode>(RG_RENDERER,
										   "common/tile/mode",
										   TM_LINEAR);
}

uint32 RenderSettings::filmWidth() const
{
	return std::max<uint32>(1,
							mRegistry->getByGroup<uint32>(RG_RENDERER,
														  "film/width",
														  1920));
}

uint32 RenderSettings::filmHeight() const
{
	return std::max<uint32>(1,
							mRegistry->getByGroup<uint32>(RG_RENDERER,
														  "film/height",
														  1080));
}

float RenderSettings::cropMinX() const
{
	return std::max<float>(0, std::min<float>(1,
											  mRegistry->getByGroup<uint32>(RG_RENDERER,
																			"film/crop/min_x",
																			0)));
}

float RenderSettings::cropMaxX() const
{
	return std::max<float>(0, std::min<float>(1,
											  mRegistry->getByGroup<uint32>(RG_RENDERER,
																			"film/crop/max_x",
																			1)));
}

float RenderSettings::cropMinY() const
{
	return std::max<float>(0, std::min<float>(1,
											  mRegistry->getByGroup<uint32>(RG_RENDERER,
																			"film/crop/min_y",
																			0)));
}

float RenderSettings::cropMaxY() const
{
	return std::max<float>(0, std::min<float>(1,
											  mRegistry->getByGroup<uint32>(RG_RENDERER,
																			"film/crop/max_y",
																			1)));
}
}
