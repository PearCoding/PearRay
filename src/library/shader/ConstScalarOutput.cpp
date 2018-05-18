#include "ConstScalarOutput.h"

#include "Logger.h"

namespace PR {
ConstScalarShaderOutput::ConstScalarShaderOutput(float f)
	: ScalarShaderOutput()
	, mValue(f)
{
}

void ConstScalarShaderOutput::eval(float& f, const PR::ShaderClosure& point) const
{
	f = mValue;
}
} // namespace PR
