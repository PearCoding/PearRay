#include "ImageShadingSocket.h"

#include "Logger.h"

namespace PR {
ImageShadingSocket::ImageShadingSocket(OIIO::TextureSystem* tsys,
									   const OIIO::TextureOpt& options,
									   const std::string& filename)
	: FloatSpectralShadingSocket()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
{
	PR_ASSERT(tsys, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");

	// TODO: Use this!
	mHandle = mTextureSystem->get_texture_handle(mFilename);
	PR_ASSERT(mHandle, "Image handle should not be NULL");
}

float ImageShadingSocket::eval(const ShadingPoint& ctx) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	float res;
	OIIO::TextureOpt ops = mTextureOptions;
	ops.firstchannel	 = ctx.Ray.WavelengthIndex;
	//ops.subimage		 = ctx.PrimID;

	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.Geometry.UVW[0], ctx.Geometry.UVW[1],
								 0, 0, 0, 0,
								 1, &res)) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup texture: " << err << std::endl;
		return 0;
	}

	return res;
}

std::string ImageShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mFilename;
	return sstream.str();
}
} // namespace PR
