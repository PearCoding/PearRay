#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB_UTILS ConstSpectralShaderOutput : public PR::SpectralShaderOutput
	{
	public:
		ConstSpectralShaderOutput(const PR::Spectrum& spec);
		PR::Spectrum eval(const PR::ShaderClosure& point) override;

	private:
		PR::Spectrum mValue;
	};
}