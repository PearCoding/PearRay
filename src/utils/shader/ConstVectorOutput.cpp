#include "ConstVectorOutput.h"

#include "Logger.h"

namespace PR
{
	ConstVectorShaderOutput::ConstVectorShaderOutput(const Eigen::Vector3f& f) :
		VectorShaderOutput(), mValue(f)
	{
	}

	Eigen::Vector3f ConstVectorShaderOutput::eval(const PR::ShaderClosure& point)
	{
		return mValue;
	}
}