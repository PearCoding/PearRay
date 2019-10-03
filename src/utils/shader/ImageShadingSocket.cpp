#include "ImageShadingSocket.h"

#include "Logger.h"
#include "math/SIMD.h"

#define PR_OIIO_HAS_BATCH_SUPPORT (OIIO_VERSION_MAJOR >= 1 && OIIO_VERSION_MINOR >= 9)

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
}

float ImageShadingSocket::eval(const ShadingPoint& ctx) const
{
	float res;
	OIIO::TextureOpt ops = mTextureOptions;
	ops.firstchannel	 = ctx.Ray.WavelengthIndex;
	ops.subimage		 = ctx.PrimID;

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
} // namespace PR
