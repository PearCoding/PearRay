#pragma once

#include "shader/ShaderOutput.h"
#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageSpectrumShaderOutput : public PR::SpectrumShaderOutput {
public:
	ImageSpectrumShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
	void eval(Spectrum& spec, const PR::ShaderClosure& point) const override;
	float evalIndex(const ShaderClosure& point, uint32 index, uint32 samples) const override;

private:
	OIIO::ustring mFilename;
	mutable OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
};
} // namespace PR
