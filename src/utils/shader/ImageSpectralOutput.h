#pragma once

#include <OpenImageIO/texture.h>
#include "shader/ShaderOutput.h"

namespace PRU
{
	class PR_LIB_UTILS ImageSpectralShaderOutput : public PR::SpectralShaderOutput
	{
	public:
		ImageSpectralShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename);
		PR::Spectrum eval(const PR::SamplePoint& point) override;

	private:
		OIIO::ustring mFilename;
		OIIO::TextureOpt mTextureOptions;
		OIIO::TextureSystem* mTextureSystem;
	};
}