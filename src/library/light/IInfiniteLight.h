#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>

namespace PR {
struct ShaderClosure;
class Spectrum;
class RenderSession;

/*
 * Representing infinite lights
 * like distant lights or backgrounds
 */
class PR_LIB IInfiniteLight {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	IInfiniteLight()
		: mFrozen(false)
	{
	}
	virtual ~IInfiniteLight() {}

	struct LightSample {
		float PDF_S;// Respect to Solid Angle
		Eigen::Vector3f L;
	};
	virtual LightSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) = 0;
	virtual void apply(Spectrum& view, const Eigen::Vector3f& V, const RenderSession& session) = 0;

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
