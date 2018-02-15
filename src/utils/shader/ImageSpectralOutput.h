#pragma once

#include "shader/ShaderOutput.h"
#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageSpectrumShaderOutput : public PR::SpectrumShaderOutput {
public:
	ImageSpectrumShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
	void eval(Spectrum& spec, const PR::ShaderClosure& point) override;

private:
	OIIO::ustring mFilename;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
};
} // namespace PR
