#pragma once

#include "RenderEnums.h"
#include "registry/Registry.h"

namespace PR {
/** @brief Bridge class to extract common information from the registry. */
class PR_LIB RenderSettings {
public:
	explicit RenderSettings(const std::shared_ptr<Registry>& registry);

	// Common integrator entries
	uint64 seed() const;
	uint32 maxRayDepth() const;

	uint64 aaSampleCount() const;
	uint64 lensSampleCount() const;
	uint64 timeSampleCount() const;
	uint64 spectralSampleCount() const;
	inline uint64 samplesPerPixel() const
	{
		return aaSampleCount() * lensSampleCount() * timeSampleCount() * spectralSampleCount();
	}

	SamplerMode aaSampler() const;
	SamplerMode lensSampler() const;
	SamplerMode timeSampler() const;
	SamplerMode spectralSampler() const;

	TimeMappingMode timeMappingMode() const;
	float timeScale() const;

	std::string integratorMode() const;
	TileMode tileMode() const;

	// Film entries
	uint32 filmWidth() const;
	uint32 filmHeight() const;
	float cropMinX() const;
	float cropMaxX() const;
	float cropMinY() const;
	float cropMaxY() const;

	inline uint32 cropWidth() const
	{
		return (cropMaxX() - cropMinX()) * filmWidth();
	}

	inline uint32 cropHeight() const
	{
		return (cropMaxY() - cropMinY()) * filmHeight();
	}

	inline uint32 cropOffsetX() const
	{
		return cropMinX() * filmWidth();
	}

	inline uint32 cropOffsetY() const
	{
		return cropMinY() * filmHeight();
	}

private:
	std::shared_ptr<Registry> mRegistry;
};
}
