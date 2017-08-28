#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB_UTILS ConstSpectrumShaderOutput : public PR::SpectrumShaderOutput
	{
	public:
		ConstSpectrumShaderOutput(const PR::Spectrum& spec);
		PR::Spectrum eval(const PR::ShaderClosure& point) override;

	private:
		PR::Spectrum mValue;
	};
}