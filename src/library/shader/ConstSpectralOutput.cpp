#include "ConstSpectralOutput.h"

#include "Logger.h"

namespace PR {
ConstSpectrumShaderOutput::ConstSpectrumShaderOutput(const Spectrum& f)
	: SpectrumShaderOutput()
	, mValue(f)
{
}

void ConstSpectrumShaderOutput::eval(Spectrum& spec, const ShaderClosure& /*point*/) const
{
	spec.copyFrom(mValue);
}

float ConstSpectrumShaderOutput::evalIndex(const ShaderClosure& /*point*/, uint32 index, uint32 /*samples*/) const
{
	return mValue(index);
}
} // namespace PR
