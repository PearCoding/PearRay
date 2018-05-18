#pragma once

#include "shader/ShaderOutput.h"
#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageVectorShaderOutput : public PR::VectorShaderOutput {
public:
	ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
	void eval(Eigen::Vector3f& v, const PR::ShaderClosure& point) const override;

private:
	OIIO::ustring mFilename;
	mutable OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
};
} // namespace PR
