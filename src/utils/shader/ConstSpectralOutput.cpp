#include "ConstSpectralOutput.h"

#include "Logger.h"

using namespace PR;
namespace PRU
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