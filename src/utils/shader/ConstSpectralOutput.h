#pragma once

#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ConstSpectralShaderOutput : public PR::SpectralShaderOutput
	{
	public:
		ConstSpectralShaderOutput(const PR::Spectrum& spec);
		PR::Spectrum eval(const PR::SamplePoint& point) override;

	private:
		PR::Spectrum mValue;
	};
}