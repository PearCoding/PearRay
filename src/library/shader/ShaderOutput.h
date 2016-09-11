#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	class Ray;
	struct ShaderClosure;
	template<typename T>
	class PR_LIB_INLINE ShaderOutput
	{
		PR_CLASS_NON_COPYABLE(ShaderOutput<T>);
	public:
		ShaderOutput<T>() = default;
		virtual ~ShaderOutput<T>() {}

		virtual T eval(const ShaderClosure& point) = 0;
	};

	typedef ShaderOutput<float> ScalarShaderOutput;
	typedef ShaderOutput<Spectrum> SpectralShaderOutput;
	typedef ShaderOutput<PM::vec> VectorShaderOutput;
}