#pragma once

#include "ProjectionMap.h"

namespace PR
{
	class PR_LIB SphereMap
	{
	public:
		inline SphereMap(uint32 resTheta, uint32 resPhi) :
			mProj(resTheta*resPhi), mResTheta(resTheta), mResPhi(resPhi)
		{
		}

		inline void setProbability(float theta, float phi, float value)
		{
			uint32 i = PM::pm_ClampT<float>(theta*PM_INV_PI_F, 0, 1) * mResTheta;
			uint32 j = PM::pm_ClampT<float>(phi*PM_INV_2_PI_F, 0, 1) * mResPhi;

			mProj.setProbability(i*mResPhi + j, value);
		}

		inline float probability(float theta, float phi) const
		{
			uint32 i = PM::pm_ClampT<float>(theta*PM_INV_PI_F, 0, 1) * mResTheta;
			uint32 j = PM::pm_ClampT<float>(phi*PM_INV_2_PI_F, 0, 1) * mResPhi;

			return mProj.probability(i*mResPhi + j);
		}

		inline bool isSetup() const
		{
			return mProj.isSetup();
		}

		inline void setup()
		{
			mProj.setup();
		}

		// u1, u2 in [0, 1]
		inline PM::vec3 sample(float u1, float u2, float& pdf) const
		{
			uint32 i = mProj.sample(u1, u2, pdf);

			uint32 phiI = i % mResPhi;
			uint32 thetaI = i / mResPhi;

			float phi = PM_2_PI_F * phiI / (float)mResPhi;
			float theta = PM_PI_F * thetaI / (float)mResTheta;

			return Projection::sphere_coord(theta, phi);
		}

		// Randomize output aswell
		// u1, u2, u3, u4 in [0, 1]
		inline PM::vec3 sample(float u1, float u2, float u3, float u4, float& pdf) const
		{
			uint32 i = mProj.sample(u1, u2, pdf);

			uint32 phiI = i % mResPhi;
			uint32 thetaI = i / mResPhi;

			float phi = PM_2_PI_F * (phiI+u3-0.5f) / (float)mResPhi;
			float theta = PM_PI_F * (thetaI+u4-0.5f) / (float)mResTheta;

			return Projection::sphere_coord(theta, phi);
		}

	private:
		ProjectionMap mProj;
		uint32 mResTheta;
		uint32 mResPhi;
	};
}