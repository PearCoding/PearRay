#pragma once

#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB ConstSpectrumShaderOutput : public SpectrumShaderOutput {
public:
	explicit ConstSpectrumShaderOutput(const Spectrum& spec);
	void eval(Spectrum& spec, const ShaderClosure& point) override;

private:
	Spectrum mValue;
};
} // namespace PR
