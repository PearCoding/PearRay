#include "ConstScalarOutput.h"

#include "Logger.h"

using namespace PR;
namespace PRU
{
	ConstScalarShaderOutput::ConstScalarShaderOutput(float f) :
		ScalarShaderOutput(), mValue(f)
	{
	}

	float ConstScalarShaderOutput::eval(const PR::ShaderClosure& point)
	{
		return mValue;
	}
}