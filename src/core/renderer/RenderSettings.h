#pragma once

#include "RenderEnums.h"

namespace PR {
class IFilterFactory;
class IIntegratorFactory;
class ISamplerFactory;
class ISpectralMapperFactory;

class IFilter;
class IIntegrator;
class ISampler;
class ISpectralMapper;

class Random;
class RenderContext;

/** @brief Bridge class to combine common render settings*/
class PR_LIB_CORE RenderSettings final {
public:
	explicit RenderSettings();
	~RenderSettings();

	// Common integrator entries
	uint64 seed;
	size_t maxParallelRays;

	TimeMappingMode timeMappingMode;
	float timeScale;
	TileMode tileMode;
	bool useAdaptiveTiling;
	bool sortHits;

	float spectralStart;
	float spectralEnd;
	bool spectralMono;
	bool spectralHero; // Use spectral wavesampling

	// Film entries
	uint32 filmWidth;
	uint32 filmHeight;
	float cropMinX;
	float cropMaxX;
	float cropMinY;
	float cropMaxY;

	// Easy access
	inline uint32 cropWidth() const
	{
		return static_cast<uint32>((cropMaxX - cropMinX) * filmWidth);
	}

	inline uint32 cropHeight() const
	{
		return static_cast<uint32>((cropMaxY - cropMinY) * filmHeight);
	}

	inline uint32 cropOffsetX() const
	{
		return static_cast<uint32>(cropMinX * filmWidth);
	}

	inline uint32 cropOffsetY() const
	{
		return static_cast<uint32>(cropMinY * filmHeight);
	}

	// Factories
	std::shared_ptr<ISamplerFactory> aaSamplerFactory;
	std::shared_ptr<ISamplerFactory> lensSamplerFactory;
	std::shared_ptr<ISamplerFactory> timeSamplerFactory;
	std::shared_ptr<ISamplerFactory> spectralSamplerFactory;

	std::shared_ptr<IFilterFactory> pixelFilterFactory;
	std::shared_ptr<IIntegratorFactory> integratorFactory;

	std::shared_ptr<ISpectralMapperFactory> spectralMapperFactory;

	// Easy access
	std::shared_ptr<ISampler> createAASampler(Random& random) const;
	std::shared_ptr<ISampler> createLensSampler(Random& random) const;
	std::shared_ptr<ISampler> createTimeSampler(Random& random) const;
	std::shared_ptr<ISampler> createSpectralSampler(Random& random) const;
	std::shared_ptr<IFilter> createPixelFilter() const;
	std::shared_ptr<IIntegrator> createIntegrator() const;
	std::shared_ptr<ISpectralMapper> createSpectralMapper(RenderContext* ctx) const;
	
	uint32 maxSampleCount() const;
};
} // namespace PR
