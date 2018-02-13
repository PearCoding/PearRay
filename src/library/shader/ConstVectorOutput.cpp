#include "ConstVectorOutput.h"

#include "Logger.h"

namespace PR
{
	ConstVectorShaderOutput::ConstVectorShaderOutput(const Eigen::Vector3f& f) :
		VectorShaderOutput(), mValue(f)
	{
	}

	void ConstVectorShaderOutput::eval(Eigen::Vector3f& p, const PR::ShaderClosure& point)
	{
		p = mValue;
	}
}