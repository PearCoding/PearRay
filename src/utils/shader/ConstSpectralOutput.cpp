#include "ConstSpectralOutput.h"

#include "Logger.h"

namespace PR
{
	ConstSpectralShaderOutput::ConstSpectralShaderOutput(const PR::Spectrum& f) :
		SpectralShaderOutput(), mValue(f)
	{
	}

	PR::Spectrum ConstSpectralShaderOutput::eval(const PR::ShaderClosure& point)
	{
		return mValue;
	}
}