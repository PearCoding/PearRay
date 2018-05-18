#pragma once

#include "IFreezable.h"
#include <Eigen/Dense>

namespace PR {
struct ShaderClosure;
class Spectrum;
class RenderSession;

/*
 * Representing infinite lights
 * like distant lights or backgrounds
 */
class PR_LIB_INLINE IInfiniteLight : public IFreezable {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	inline IInfiniteLight()
		: IFreezable(){};
	virtual ~IInfiniteLight() = default;

	struct LightSample {
		float PDF_S{ 0 }; // Respect to Solid Angle
		Eigen::Vector3f L;
	};
	virtual LightSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) = 0;
	virtual void apply(Spectrum& view, const Eigen::Vector3f& V, const RenderSession& session)						 = 0;
};
} // namespace PR
