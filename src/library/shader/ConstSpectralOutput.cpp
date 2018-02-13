#include "ConstSpectralOutput.h"

#include "Logger.h"

namespace PR
{
	ConstSpectrumShaderOutput::ConstSpectrumShaderOutput(const Spectrum& f) :
		SpectrumShaderOutput(), mValue(f)
	{
	}

	void ConstSpectrumShaderOutput::eval(Spectrum& spec, const ShaderClosure& point)
	{
		spec = mValue;
	}
}