#include "ConstVectorOutput.h"

#include "Logger.h"

using namespace PR;
namespace PRU
{
	ConstVectorShaderOutput::ConstVectorShaderOutput(const PM::vec3& f) :
		VectorShaderOutput(), mValue(f)
	{
	}

	PM::vec3 ConstVectorShaderOutput::eval(const PR::ShaderClosure& point)
	{
		return mValue;
	}
}