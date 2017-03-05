#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB_UTILS ConstVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ConstVectorShaderOutput(const PM::vec3& spec);
		PM::vec3 eval(const PR::ShaderClosure& point) override;

	private:
		PM::vec3 mValue;
	};
}