#pragma once

#include "RenderEnums.h"

namespace PR {
/** @brief Bridge class to extract common information from the registry. */
class PR_LIB RenderSettings {
public:
	explicit RenderSettings();

	// Common integrator entries
	uint64 seed;
	uint32 maxRayDepth;

	uint64 aaSampleCount;
	uint64 lensSampleCount;
	uint64 timeSampleCount;
	uint64 spectralSampleCount;

	SamplerMode aaSampler;
	SamplerMode lensSampler;
	SamplerMode timeSampler;
	SamplerMode spectralSampler;

	SpectralProcessMode spectralProcessMode;

	TimeMappingMode timeMappingMode;
	float timeScale;
	TileMode tileMode;

	// Film entries
	uint32 filmWidth;
	uint32 filmHeight;
	float cropMinX;
	float cropMaxX;
	float cropMinY;
	float cropMaxY;

	// Easy access
	inline uint64 samplesPerPixel() const
	{
		return aaSampleCount * lensSampleCount * timeSampleCount * spectralSampleCount;
	}

	inline uint32 cropWidth() const
	{
		return (cropMaxX - cropMinX) * filmWidth;
	}

	inline uint32 cropHeight() const
	{
		return (cropMaxY - cropMinY) * filmHeight;
	}

	inline uint32 cropOffsetX() const
	{
		return cropMinX * filmWidth;
	}

	inline uint32 cropOffsetY() const
	{
		return cropMinY * filmHeight;
	}
};
} // namespace PR
