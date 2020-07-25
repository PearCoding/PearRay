#pragma once

#include "ElevationAzimuth.h"
#include "SkySunConfig.h"

namespace PR {
class FloatSpectralNode;
struct ElevationAzimuth;
class ParameterGroup;
class SkyModel {
public:
	SkyModel(const std::shared_ptr<FloatSpectralNode>& ground_albedo, const ElevationAzimuth& sunEA, const ParameterGroup& params);

	inline size_t azimuthCount() const { return mAzimuthCount; }
	inline size_t elevationCount() const { return mElevationCount; }

	inline float radiance(int wvl_band, const ElevationAzimuth& ea) const
	{
		int az_in = std::max(0, std::min<int>(mAzimuthCount - 1, int(ea.Azimuth / AZIMUTH_RANGE * mAzimuthCount)));
		int el_in = std::max(0, std::min<int>(mElevationCount - 1, int(ea.Elevation / ELEVATION_RANGE * mElevationCount)));
		return mData[el_in * mAzimuthCount * AR_SPECTRAL_BANDS + az_in * AR_SPECTRAL_BANDS + wvl_band];
	}

private:
	std::vector<float> mData;

	size_t mAzimuthCount;
	size_t mElevationCount;
};
} // namespace PR