#include "ImageSpectralOutput.h"

#include "Logger.h"
#include "shader/ShaderClosure.h"

#include "spectral/RGBConverter.h"

namespace PR {
ImageSpectrumShaderOutput::ImageSpectrumShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename)
	: SpectrumShaderOutput()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
}

void ImageSpectrumShaderOutput::eval(Spectrum& spec, const PR::ShaderClosure& point) const
{
	float res[3] = { 0, 0, 0 };

	if (!mTextureSystem->texture(mFilename, mTextureOptions,
								 point.UVW(0), 1 - point.UVW(1),
								 point.dUVWdX(0), point.dUVWdY(0),
								 point.dUVWdX(1), point.dUVWdY(1),
								 3, &res[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup texture at UV [" << point.UVW << "]: " << err << std::endl;
	}

	RGBConverter::toSpec(spec, res[0], res[1], res[2]);
}

float ImageSpectrumShaderOutput::evalIndex(const ShaderClosure& point, uint32 index, uint32 samples) const
{
	float res[3] = { 0, 0, 0 };

	if (!mTextureSystem->texture(mFilename, mTextureOptions,
								 point.UVW(0), 1 - point.UVW(1),
								 point.dUVWdX(0), point.dUVWdY(0),
								 point.dUVWdX(1), point.dUVWdY(1),
								 3, &res[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup texture at UV [" << point.UVW << "]: " << err << std::endl;
	}

	return RGBConverter::toSpecIndex(samples, index, res[0], res[1], res[2]);
}

} // namespace PR
