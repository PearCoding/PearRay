#pragma once

#include "ProjectionMap.h"

#include <Eigen/Dense>

namespace PR {
class PR_LIB SphereMap {
public:
	inline SphereMap(uint32 resTheta, uint32 resPhi)
		: mProj(resTheta * resPhi)
		, mResTheta(resTheta)
		, mResPhi(resPhi)
	{
	}

	inline void setProbabilityWithIndex(uint32 thetaI, uint32 phiI, float value)
	{
		mProj.setProbability(thetaI * mResPhi + phiI, value);
	}

	inline float probabilityWithIndex(uint32 thetaI, uint32 phiI) const
	{
		return mProj.probability(thetaI * mResPhi + phiI);
	}

	inline void setProbability(float theta, float phi, float value)
	{
		uint32 i = std::min<float>(std::max<float>(theta * PR_1_PI, 0), 1) * mResTheta;
		uint32 j = std::min<float>(std::max<float>(phi * 0.5 * PR_1_PI, 0), 1) * mResPhi;

		setProbabilityWithIndex(i, j, value);
	}

	inline float probability(float theta, float phi) const
	{
		uint32 i = std::min<float>(std::max<float>(theta * PR_1_PI, 0), 1) * mResTheta;
		uint32 j = std::min<float>(std::max<float>(phi * 0.5 * PR_1_PI, 0), 1) * mResPhi;

		return probabilityWithIndex(i, j);
	}

	inline bool isSetup() const
	{
		return mProj.isSetup();
	}

	inline void setup()
	{
		mProj.rebound();
		mProj.scale(PR_1_PI * 0.25f);
		mProj.setup();
	}

	// u1, u2 in [0, 1]
	inline Eigen::Vector3f sample(float u1, float u2, float& pdf) const
	{
		uint32 i = mProj.sample(u1, u2, pdf);

		uint32 phiI   = i % mResPhi;
		uint32 thetaI = i / mResPhi;

		float phi   = 2 * PR_PI * phiI / static_cast<float>(mResPhi);
		float theta = PR_PI * thetaI / static_cast<float>(mResTheta);

		return Projection::sphere_coord(theta, phi);
	}

	// Randomize output aswell
	// u1, u2, u3, u4 in [0, 1]
	inline Eigen::Vector3f sample(float u1, float u2, float u3, float u4, float& pdf) const
	{
		uint32 i = mProj.sample(u1, u2, pdf);

		uint32 phiI   = i % mResPhi;
		uint32 thetaI = i / mResPhi;

		float phi   = 2 * PR_PI * std::min<float>(std::max<float>((phiI + u3) / static_cast<float>(mResPhi), 0.0f), 1.0f);
		float theta = PR_PI * std::min<float>(std::max<float>((thetaI + u4) / static_cast<float>(mResTheta), 0.0f), 1.0f);

		return Projection::sphere_coord(theta, phi);
	}

private:
	ProjectionMap mProj;
	uint32 mResTheta;
	uint32 mResPhi;
};
} // namespace PR
