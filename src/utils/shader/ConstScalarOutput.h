#pragma once

#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ConstScalarShaderOutput : public PR::ScalarShaderOutput
	{
	public:
		ConstScalarShaderOutput(float f);
		float eval(const PR::ShaderClosure& point) override;

	private:
		float mValue;
	};
}