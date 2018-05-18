#pragma once

#include "shader/ShaderOutput.h"
#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageScalarShaderOutput : public PR::ScalarShaderOutput {
public:
	ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
	void eval(float& f, const PR::ShaderClosure& point) const override;

private:
	OIIO::ustring mFilename;
	mutable OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
};
} // namespace PR
