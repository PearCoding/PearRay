#pragma once

#include "spectral/Spectrum.h"
#include <Eigen/Dense>
#include <type_traits>

namespace PR {
class Ray;
struct ShaderClosure;
template <typename T>
class PR_LIB_INLINE ShaderOutput {
	PR_CLASS_NON_COPYABLE(ShaderOutput<T>);

public:
	ShaderOutput<T>() = default;
	virtual ~ShaderOutput<T>() {}

	virtual void eval(T& t, const ShaderClosure& point) = 0;

	template<class Q = T>
	inline typename std::enable_if<!std::is_same<Q, Spectrum>::value, Q>::type eval(const ShaderClosure& point) {
		Q val;
		eval(val, point);
		return val;
	}
};

typedef ShaderOutput<float> ScalarShaderOutput;
typedef ShaderOutput<Spectrum> SpectrumShaderOutput;
typedef ShaderOutput<Eigen::Vector3f> VectorShaderOutput;
}
