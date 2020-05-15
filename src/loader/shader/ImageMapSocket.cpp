#include "ImageMapSocket.h"
#include "spectral/SpectralUpsampler.h"

#include "Logger.h"

namespace PR {
ImageMapSocket::ImageMapSocket(OIIO::TextureSystem* tsys,
							   const OIIO::TextureOpt& options,
							   const std::string& filename)
	: FloatSpectralMapSocket()
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
		if (spec->get_string_attribute("oiio.ColorSpace") != "PRParametric")
			PR_LOG(L_WARNING) << "Image " << mFilename << " is not parametric " << std::endl;

		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::STRING);
		if (ptex && ptex->type() == OIIO::TypeDesc::STRING) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

SpectralBlob ImageMapSocket::eval(const MapSocketCoord& ctx) const
{
	SpectralBlob value;
	lookup(ctx, value);
	return value;
}

float ImageMapSocket::relativeLuminance(const MapSocketCoord& ctx) const
{
	// TODO
	SpectralBlob value;
	lookup(ctx, value);
	return value(0);
}

void ImageMapSocket::lookup(const MapSocketCoord& ctx, SpectralBlob& rgb) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = static_cast<int>(ctx.Face);

	ParametricBlob value;
	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.UV(0), 1 - ctx.UV(1),
								 ctx.dUV(0), ctx.dUV(1), ctx.dUV(0), ctx.dUV(1),
								 3, &value[0])) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup luminance of texture: " << err << std::endl;
		rgb = SpectralBlob::Zero();
		return;
	}

	rgb = SpectralUpsampler::compute(value, ctx.WavelengthNM);
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
	if (mIsPtex)
		stream << " [Ptex]";
	return stream.str();
}
} // namespace PR
