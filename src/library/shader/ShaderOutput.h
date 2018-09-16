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

	virtual void eval(T& t, const ShaderClosure& point) const = 0;

	template<class Q = T>
	inline typename std::enable_if<!std::is_same<Q, Spectrum>::value, Q>::type eval(const ShaderClosure& point) const {
		Q val;
		eval(val, point);
		return val;
	}
};

typedef ShaderOutput<float> ScalarShaderOutput;
typedef ShaderOutput<Eigen::Vector3f> VectorShaderOutput;

class PR_LIB_INLINE SpectrumShaderOutput : public ShaderOutput<Spectrum> {
	PR_CLASS_NON_COPYABLE(SpectrumShaderOutput);
public:
	SpectrumShaderOutput() = default;
	virtual ~SpectrumShaderOutput() {}

	virtual float evalIndex(const ShaderClosure& point, uint32 index, uint32 samples) const = 0;// Fast access
};
}
