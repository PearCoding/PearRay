#pragma once

#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ConstVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ConstVectorShaderOutput(const PM::vec& spec);
		PM::vec eval(const PR::SamplePoint& point) override;

	private:
		PM::vec mValue;
	};
}