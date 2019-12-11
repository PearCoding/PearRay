#pragma once

#include "RenderEnums.h"

namespace PR {
class IFilter;

/** @brief Bridge class to extract common information from the registry. */
class PR_LIB RenderSettings {
public:
	explicit RenderSettings();

	// Common integrator entries
	uint64 seed;
	uint32 maxRayDepth;
	size_t maxParallelRays;

	uint32 aaSampleCount;
	uint32 lensSampleCount;
	uint32 timeSampleCount;

	SamplerMode aaSampler;
	SamplerMode lensSampler;
	SamplerMode timeSampler;

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
	inline uint32 samplesPerPixel() const
	{
		return aaSampleCount * lensSampleCount * timeSampleCount;
	}

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

	size_t pixelFilterRadius;
	std::string pixelFilter;
	std::shared_ptr<IFilter> createPixelFilter() const;
};
} // namespace PR
