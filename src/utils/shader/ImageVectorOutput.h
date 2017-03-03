#pragma once

#include <OpenImageIO/texture.h>
#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ImageVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
		PM::vec3 eval(const PR::ShaderClosure& point) override;

	private:
		OIIO::ustring mFilename;
		OIIO::TextureOpt mTextureOptions;
		OIIO::TextureSystem* mTextureSystem;
	};
}