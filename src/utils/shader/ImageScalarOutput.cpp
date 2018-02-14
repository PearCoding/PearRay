#include "ImageScalarOutput.h"

#include "shader/ShaderClosure.h"
#include "Logger.h"

namespace PR
{
	ImageScalarShaderOutput::ImageScalarShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		ScalarShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(tsys, "Given texture system has to be valid");
		PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
	}

	void ImageScalarShaderOutput::eval(float& res, const PR::ShaderClosure& point)
	{
		res = 0;
		if (!mTextureSystem->texture(mFilename, mTextureOptions,
			point.UVW(0), 1 - point.UVW(1),
			point.dUVWdX(0), point.dUVWdY(0),
			point.dUVWdX(1), point.dUVWdY(1),
			1, &res))
		{
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				point.UVW(0), 1 - point.UVW(1), err.c_str());
		}
	}
}