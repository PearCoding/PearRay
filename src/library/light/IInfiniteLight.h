#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>

namespace PR {
struct ShaderClosure;
class Spectrum;

/*
 * Representing infinite lights
 * like distant lights or backgrounds
 */
class PR_LIB IInfiniteLight {
public:
	IInfiniteLight()
		: mFrozen(false)
	{
	}
	virtual ~IInfiniteLight() {}

	virtual Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) = 0;
	virtual Spectrum apply(const Eigen::Vector3f& V) = 0;

	inline void freeze()
	{
		if (!mFrozen) {
			onFreeze();
			mFrozen = true;
		}
	}

	inline bool isFrozen() const
	{
		return mFrozen;
	}

	virtual void onFreeze() {}

private:
	bool mFrozen;
};
}
