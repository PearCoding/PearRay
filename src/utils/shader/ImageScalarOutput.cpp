#include "ImageScalarOutput.h"

#include "shader/SamplePoint.h"
#include "Logger.h"

using namespace PR;
namespace PRU
{
	ImageScalarShaderOutput::ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		ScalarShaderOutput(), mTexture(nullptr), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys);
		PR_ASSERT(!filename.empty());

		mTexture = mTextureSystem->get_texture_handle(OIIO::ustring(filename));
	}

	float ImageScalarShaderOutput::eval(const PR::SamplePoint& point)
	{
		float res = 0;
		if (!mTextureSystem->texture(mTexture, nullptr, mTextureOptions,
			PM::pm_GetX(point.UV), PM::pm_GetY(point.UV),
			0, 0, 0, 0,
			1, &res))
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]",
				PM::pm_GetX(point.UV), PM::pm_GetY(point.UV));
		}

		return res;
	}
}