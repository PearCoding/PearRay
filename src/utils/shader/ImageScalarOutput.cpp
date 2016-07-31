#include "ImageScalarOutput.h"

#include "shader/SamplePoint.h"
#include "Logger.h"

using namespace PR;
namespace PRU
{
	ImageScalarShaderOutput::ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		ScalarShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys);
		PR_ASSERT(!mFilename.empty());
	}

	float ImageScalarShaderOutput::eval(const PR::SamplePoint& point)
	{
		float res = 0;
		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			PM::pm_GetX(point.UV), 1 - PM::pm_GetY(point.UV),
			0, 0, 0, 0,
			1, &res))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UV), 1 - PM::pm_GetY(point.UV), err.c_str());
		}

		return res;
	}
}