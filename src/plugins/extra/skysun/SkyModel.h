#pragma once

#include "SkySunConfig.h"

namespace PR {
class FloatSpectralNode;
struct ElevationAzimuth;
class ParameterGroup;
class SkyModel {
public:
	SkyModel(const std::shared_ptr<FloatSpectralNode>& ground_albedo, const ElevationAzimuth& sunEA, const ParameterGroup& params);

	inline size_t phiCount() const { return mPhiCount; }
	inline size_t thetaCount() const { return mThetaCount; }

	inline float radiance(int wvl_band, float u, float v) const
	{
		int x = std::max(0, std::min<int>(mPhiCount - 1, int(u * mPhiCount)));
		int y = std::max(0, std::min<int>(mThetaCount - 1, int(v * mThetaCount)));
		return mData[y * mPhiCount * AR_SPECTRAL_BANDS + x * AR_SPECTRAL_BANDS + wvl_band];
	}

private:
	std::vector<float> mData;

	size_t mPhiCount;
	size_t mThetaCount;
};
} // namespace PR