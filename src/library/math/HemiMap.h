#pragma once

#include "ProjectionMap.h"

namespace PR {
class PR_LIB HemiMap {
public:
	inline HemiMap(uint32 resTheta, uint32 resPhi)
		: mProj(resTheta * resPhi)
		, mResTheta(resTheta)
		, mResPhi(resPhi)
	{
	}

	inline void setProbabilityWithIndex(uint32 i, uint32 j, float value)
	{
		mProj.setProbability(i * mResPhi + j, value);
	}

	inline float probabilityWithIndex(uint32 i, uint32 j) const
	{
		return mProj.probability(i * mResPhi + j);
	}

	inline void setProbability(float theta, float phi, float value)
	{
		uint32 i = PM::pm_ClampT<float>(2 * theta * PR_1_PI, 0, 1) * mResTheta;
		uint32 j = PM::pm_ClampT<float>(phi * PM_INV_2_PI_F, 0, 1) * mResPhi;

		setProbabilityWithIndex(i, j, value);
	}

	inline float probability(float theta, float phi) const
	{
		uint32 i = PM::pm_ClampT<float>(2 * theta * PR_1_PI, 0, 1) * mResTheta;
		uint32 j = PM::pm_ClampT<float>(phi * PM_INV_2_PI_F, 0, 1) * mResPhi;

		return probabilityWithIndex(i, j);
	}

	inline bool isSetup() const
	{
		return mProj.isSetup();
	}

	inline void setup()
	{
		mProj.rebound();
		mProj.scale(PM_INV_2_PI_F);
		mProj.setup();
	}

	// u1, u2 in [0, 1]
	inline Vector3f sample(float u1, float u2, float& pdf) const
	{
		uint32 i = mProj.sample(u1, u2, pdf);

		uint32 phiI   = i % mResPhi;
		uint32 thetaI = i / mResPhi;

		float phi   = PM_2_PI_F * phiI / static_cast<float>(mResPhi);
		float theta = PM_PI_2_DIV_F * thetaI / static_cast<float>(mResTheta);

		return Projection::sphere_coord(theta, phi);
	}

	// Randomize output aswell
	// u1, u2, u3, u4 in [0, 1]
	inline Vector3f sample(float u1, float u2, float u3, float u4, float& pdf) const
	{
		uint32 i = mProj.sample(u1, u2, pdf);

		uint32 phiI   = i % mResPhi;
		uint32 thetaI = i / mResPhi;

		float phi   = PM_2_PI_F * PM::pm_ClampT((phiI + u3) / static_cast<float>(mResPhi), 0.0f, 1.0f);
		float theta = PM_PI_2_DIV_F * PM::pm_ClampT((thetaI + u4) / static_cast<float>(mResTheta), 0.0f, 1.0f);

		return Projection::sphere_coord(theta, phi);
	}

private:
	ProjectionMap mProj;
	uint32 mResTheta;
	uint32 mResPhi;
};
} // namespace PR
