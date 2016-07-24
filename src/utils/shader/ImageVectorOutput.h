#pragma once

#include <OpenImageIO/texture.h>
#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ImageVectorShaderOutput : public PR::VectorShaderOutput
	{
	public:
		ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
		PM::vec eval(const PR::SamplePoint& point) override;

	private:
		OIIO::TextureSystem::TextureHandle* mTexture;
		OIIO::TextureOpt mTextureOptions;
		OIIO::TextureSystem* mTextureSystem;
	};
}