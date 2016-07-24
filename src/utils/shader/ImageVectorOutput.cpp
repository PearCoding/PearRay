#include "ImageVectorOutput.h"

#include "shader/SamplePoint.h"
#include "Logger.h"

using namespace PR;
namespace PRU
{
	ImageVectorShaderOutput::ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		VectorShaderOutput(), mTexture(nullptr), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys);
		PR_ASSERT(!filename.empty());

		mTexture = mTextureSystem->get_texture_handle(OIIO::ustring(filename));
	}

	PM::vec ImageVectorShaderOutput::eval(const PR::SamplePoint& point)
	{
		float res[4] = { 0, 0, 0, 0 };
		if (!mTextureSystem->texture(mTexture, nullptr, mTextureOptions,
			PM::pm_GetX(point.UV), PM::pm_GetY(point.UV),
			0, 0, 0, 0,
			4, &res[0]))
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]",
				PM::pm_GetX(point.UV), PM::pm_GetY(point.UV));
		}

		return PM::pm_Set(res[0], res[1], res[2], res[3]);
	}
}