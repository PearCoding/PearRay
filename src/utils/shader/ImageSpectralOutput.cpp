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
		PR_ASSERT(mTextureSystem, "Given texture system has to be valid");
		PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
	}

	PR::Spectrum ImageSpectralShaderOutput::eval(const PR::ShaderClosure& point)
	{
		float res[3] = { 0, 0, 0 };

		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			PM::pm_GetX(point.UVW), 1-PM::pm_GetY(point.UVW),
			PM::pm_GetX(point.dUVWdX), PM::pm_GetX(point.dUVWdY),
			PM::pm_GetY(point.dUVWdX), PM::pm_GetY(point.dUVWdY),
			3, &res[0]))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UVW), 1-PM::pm_GetY(point.UVW), err.c_str());
		}
		
		return RGBConverter::toSpec(res[0], res[1], res[2]);
	}
}