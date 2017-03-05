#include "ConstVectorOutput.h"

#include "Logger.h"

namespace PR
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