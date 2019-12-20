#include "ImageMapSocket.h"
#include "spectral/RGBConverter.h"

#include "Logger.h"

namespace PR {
ImageMapSocket::ImageMapSocket(const std::shared_ptr<SpectrumDescriptor>& desc,
							   OIIO::TextureSystem* tsys,
							   const OIIO::TextureOpt& options,
							   const std::string& filename)
	: FloatSpectralMapSocket()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
	, mImageDescriptor(SpectrumDescriptor::createSRGBTriplet())
	, mEngineDescriptor(desc)
	, mIsPtex(false)
	, mIsLinear(true)
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
		if (spec->get_string_attribute("oiio.ColorSpace") == "sRGB")
			mIsLinear = false;

		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::TypeString);
		if (ptex && ptex->type() == OIIO::TypeDesc::TypeString) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

ColorTriplet ImageMapSocket::eval(const MapSocketCoord& ctx) const
{
	ColorTriplet rgb;
	lookup(ctx, rgb);
	return mEngineDescriptor->convertTriplet(mImageDescriptor, rgb);
}

float ImageMapSocket::relativeLuminance(const MapSocketCoord& ctx) const
{
	// TODO
	ColorTriplet rgb;
	lookup(ctx, rgb);
	return RGBConverter::luminance(rgb[0], rgb[1], rgb[2]);
}

void ImageMapSocket::lookup(const MapSocketCoord& ctx, ColorTriplet& rgb) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = static_cast<int>(ctx.Face);

	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.UV(0), 1 - ctx.UV(1),
								 ctx.dUV(0), ctx.dUV(1), ctx.dUV(0), ctx.dUV(1),
								 3, &rgb[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup luminance of texture: " << err << std::endl;
		rgb[0] = 0.0f;
		rgb[1] = 0.0f;
		rgb[2] = 0.0f;
		return;
	}

	if (!mIsLinear)
		RGBConverter::linearize(rgb[0], rgb[1], rgb[2]);
}

Vector2i ImageMapSocket::queryRecommendedSize() const
{
	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (spec) {
		return Vector2i(spec->width, spec->height);
	} else {
		return Vector2i(1, 1);
	}
}

std::string ImageMapSocket::dumpInformation() const
{
	std::stringstream stream;
	stream << mFilename;
	if (!mIsLinear)
		stream << " [NonLinear]";
	if (mIsPtex)
		stream << " [Ptex]";
	return stream.str();
}
} // namespace PR
