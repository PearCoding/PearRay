#include "ImageScalarOutput.h"

#include "shader/ShaderClosure.h"
#include "Logger.h"

using namespace PR;
namespace PRU
{
	ImageScalarShaderOutput::ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		ScalarShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys, "Given texture system has to be valid");
		PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
	}

	float ImageScalarShaderOutput::eval(const PR::ShaderClosure& point)
	{
		float res = 0;
		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			PM::pm_GetX(point.UVW), 1 - PM::pm_GetY(point.UVW),
			PM::pm_GetX(point.dUVWdX), PM::pm_GetX(point.dUVWdY),
			PM::pm_GetY(point.dUVWdX), PM::pm_GetY(point.dUVWdY),
			1, &res))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UVW), 1 - PM::pm_GetY(point.UVW), err.c_str());
		}

		return res;
	}
}