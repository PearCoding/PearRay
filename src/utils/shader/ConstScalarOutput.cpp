#include "ConstScalarOutput.h"

#include "Logger.h"

namespace PR
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