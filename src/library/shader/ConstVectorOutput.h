#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB ConstVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ConstVectorShaderOutput(const Eigen::Vector3f& spec);
		void eval(Eigen::Vector3f& p, const PR::ShaderClosure& point) override;

	private:
		Eigen::Vector3f mValue;
	};
}