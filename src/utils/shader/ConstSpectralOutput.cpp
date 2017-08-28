#include "ConstSpectralOutput.h"

#include "Logger.h"

namespace PR
{
	ConstSpectrumShaderOutput::ConstSpectrumShaderOutput(const PR::Spectrum& f) :
		SpectrumShaderOutput(), mValue(f)
	{
	}

	PR::Spectrum ConstSpectrumShaderOutput::eval(const PR::ShaderClosure& point)
	{
		return mValue;
	}
}