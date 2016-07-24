#pragma once

#include <OpenImageIO/texture.h>
#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ImageScalarShaderOutput : public PR::ScalarShaderOutput
	{
	public:
		ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
		float eval(const PR::SamplePoint& point) override;

	private:
		OIIO::TextureSystem::TextureHandle* mTexture;
		OIIO::TextureOpt mTextureOptions;
		OIIO::TextureSystem* mTextureSystem;
	};
}