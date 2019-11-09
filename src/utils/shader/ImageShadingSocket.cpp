#include "ImageShadingSocket.h"
#include "spectral/RGBConverter.h"

#include "Logger.h"

namespace PR {
ImageShadingSocket::ImageShadingSocket(OIIO::TextureSystem* tsys,
									   const OIIO::TextureOpt& options,
									   const std::string& filename)
	: FloatSpectralShadingSocket()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
	, mIsPtex(false)
{
	PR_ASSERT(tsys, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");

	// TODO: Use this!
	mHandle = mTextureSystem->get_texture_handle(mFilename);
	PR_ASSERT(mHandle, "Image handle should not be NULL");

	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (!spec) {
		PR_LOG(L_FATAL) << "Couldn't lookup texture specification of image " << mFilename << std::endl;
	} else {
		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::TypeString);
		if (ptex && ptex->type() == OIIO::TypeDesc::TypeString) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

float ImageShadingSocket::eval(const ShadingPoint& ctx) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	float res;
	OIIO::TextureOpt ops = mTextureOptions;
	ops.firstchannel	 = ctx.Ray.WavelengthIndex;

	if (mIsPtex)
		ops.subimage = ctx.PrimID;

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

float ImageShadingSocket::relativeLuminance(const ShadingPoint& ctx) const
{
	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = ctx.PrimID;

	float rgb[3];
	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.Geometry.UVW[0], ctx.Geometry.UVW[1],
								 0, 0, 0, 0,
								 3, &rgb[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup luminance of texture: " << err << std::endl;
		return 0;
	}

	return RGBConverter::luminance(rgb[0], rgb[1], rgb[2]);
}

std::string ImageShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mFilename;
	return sstream.str();
}
} // namespace PR
