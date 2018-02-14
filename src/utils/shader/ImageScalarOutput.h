#pragma once

#include <OpenImageIO/texture.h>
#include "shader/ShaderOutput.h"

namespace PR
{
	class PR_LIB_UTILS ImageScalarShaderOutput : public PR::ScalarShaderOutput
	{
	public:
		ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
		void eval(float& f, const PR::ShaderClosure& point) override;

	private:
		OIIO::ustring mFilename;
		OIIO::TextureOpt mTextureOptions;
		OIIO::TextureSystem* mTextureSystem;
	};
}