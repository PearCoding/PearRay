#include "ImageVectorOutput.h"

#include "shader/ShaderClosure.h"
#include "Logger.h"

using namespace PR;
namespace PRU
{
	ImageVectorShaderOutput::ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		VectorShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys, "Given texture system has to be valid");
		PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
	}

	PM::vec3 ImageVectorShaderOutput::eval(const PR::ShaderClosure& point)
	{
		float res[4] = { 0, 0, 0};
		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			PM::pm_GetX(point.UVW), 1 - PM::pm_GetY(point.UVW),
			PM::pm_GetX(point.dUVWdX), PM::pm_GetX(point.dUVWdY),
			PM::pm_GetY(point.dUVWdX), PM::pm_GetY(point.dUVWdY),
			4, &res[0]))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UVW), 1 - PM::pm_GetY(point.UVW), err.c_str());
		}

		// TODO
		return PM::pm_Set(res[0], res[1], res[2]);
	}
}