#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB_UTILS ConstVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ConstVectorShaderOutput(const Eigen::Vector3f& spec);
		Eigen::Vector3f eval(const PR::ShaderClosure& point) override;

	private:
		Eigen::Vector3f mValue;
	};
}