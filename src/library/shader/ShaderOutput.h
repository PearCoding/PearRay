#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

namespace PR
{
	class Ray;
	struct SamplePoint;
	template<typename T>
	class PR_LIB_INLINE ShaderOutput
	{
		PR_CLASS_NON_COPYABLE(ShaderOutput<T>);
	public:
		ShaderOutput<T>() = default;

		virtual T eval(const SamplePoint& point) = 0;
	};

	typedef ShaderOutput<float> ScalarShaderOutput;
	typedef ShaderOutput<Spectrum> SpectralShaderOutput;
	typedef ShaderOutput<PM::vec> VectorShaderOutput;
}