#include "ImageSpectralOutput.h"

#include "shader/ShaderClosure.h"
#include "Logger.h"

#include "spectral/RGBConverter.h"

using namespace PR;
namespace PRU
{
	ImageSpectralShaderOutput::ImageSpectralShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		SpectralShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(mTextureSystem);
		PR_ASSERT(!mFilename.empty());
	}

	PR::Spectrum ImageSpectralShaderOutput::eval(const PR::ShaderClosure& point)
	{
		float res[3] = { 0, 0, 0 };

		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			PM::pm_GetX(point.UV), 1-PM::pm_GetY(point.UV),
			PM::pm_GetX(point.dUVdX), PM::pm_GetX(point.dUVdY),
			PM::pm_GetY(point.dUVdX), PM::pm_GetY(point.dUVdY),
			3, &res[0]))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UV), 1-PM::pm_GetY(point.UV), err.c_str());
		}
		
		return RGBConverter::toSpec(res[0], res[1], res[2]);
	}
}