#include "ImageVectorOutput.h"

#include "Logger.h"
#include "shader/ShaderClosure.h"

namespace PR {
ImageVectorShaderOutput::ImageVectorShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename)
	: VectorShaderOutput()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
{
	PR_ASSERT(tsys, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
}

void ImageVectorShaderOutput::eval(Eigen::Vector3f& v, const PR::ShaderClosure& point) const
{
	float res[4] = { 0, 0, 0 };
	if (!mTextureSystem->texture(mFilename, mTextureOptions,
								 point.UVW(0), 1 - point.UVW(1),
								 point.dUVWdX(0), point.dUVWdY(0),
								 point.dUVWdX(1), point.dUVWdY(1),
								 4, &res[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup texture at UV [" << point.UVW << "]: " << err << std::endl;
	}

	// TODO
	v = Eigen::Vector3f(res[0], res[1], res[2]);
}
} // namespace PR
