#include "ImageNode.h"
#include "spectral/SpectralUpsampler.h"

#include "Logger.h"

namespace PR {
//// Parametric
ParametricImageNode::ParametricImageNode(OIIO::TextureSystem* tsys,
										 const OIIO::TextureOpt& options,
										 const std::string& filename)
	: FloatSpectralNode()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
	, mIsPtex(false)
	, mErrorIdenticator(false)
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
		if (spec->get_string_attribute("PR:Parametric") != "true")
			PR_LOG(L_WARNING) << "Image " << mFilename << " is not parametric" << std::endl;

		if (spec->format != OIIO::TypeFloat)
			PR_LOG(L_WARNING) << "Image " << mFilename << " is parametric but not of base type float" << std::endl;

		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::STRING);
		if (ptex && ptex->type() == OIIO::TypeDesc::STRING) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

SpectralBlob ParametricImageNode::eval(const ShadingContext& ctx) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = static_cast<int>(ctx.Face);

	ParametricBlob value;
	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.UV(0), 1 - ctx.UV(1),
								 0.0f, 0.0f, 0.0f, 0.0f,
								 /*ctx.dUV(0), ctx.dUV(1), ctx.dUV(0), ctx.dUV(1)*/
								 PR_PARAMETRIC_BLOB_SIZE, &value[0])) {

		if (!mErrorIdenticator.exchange(true)) {
			const std::string err = mTextureSystem->geterror();
			PR_LOG(L_ERROR) << "Could not lookup texture [" << mFilename << "]: " << err << std::endl;
		}

		return SpectralBlob::Zero();
	}

	return SpectralUpsampler::compute(value, ctx.WavelengthNM);
}

Vector2i ParametricImageNode::queryRecommendedSize() const
{
	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (spec) {
		return Vector2i(spec->width, spec->height);
	} else {
		return Vector2i(1, 1);
	}
}

std::string ParametricImageNode::dumpInformation() const
{
	std::stringstream stream;
	stream << mFilename;
	if (mIsPtex)
		stream << " [Ptex]";
	return stream.str();
}
//// Non-Parametric
NonParametricImageNode::NonParametricImageNode(OIIO::TextureSystem* tsys,
											   const OIIO::TextureOpt& options,
											   const std::string& filename,
											   SpectralUpsampler* upsampler)
	: FloatSpectralNode()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
	, mUpsampler(upsampler)
	, mIsPtex(false)
	, mErrorIdenticator(false)
{
	PR_ASSERT(tsys, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
	PR_ASSERT(upsampler, "Given spectral upsampler has to be valid");

	// TODO: Use this!
	mHandle = mTextureSystem->get_texture_handle(mFilename);
	PR_ASSERT(mHandle, "Image handle should not be NULL");

	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (!spec) {
		PR_LOG(L_FATAL) << "Couldn't lookup texture specification of image " << mFilename << std::endl;
	} else {
		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::STRING);
		if (ptex && ptex->type() == OIIO::TypeDesc::STRING) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

SpectralBlob NonParametricImageNode::eval(const ShadingContext& ctx) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = static_cast<int>(ctx.Face);

	std::array<float, 3> value;
	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.UV(0), 1 - ctx.UV(1),
								 0.0f, 0.0f, 0.0f, 0.0f,
								 /*ctx.dUV(0), ctx.dUV(1), ctx.dUV(0), ctx.dUV(1)*/
								 3, &value[0])) {

		if (!mErrorIdenticator.exchange(true)) {
			const std::string err = mTextureSystem->geterror();
			PR_LOG(L_ERROR) << "Could not lookup texture [" << mFilename << "]: " << err << std::endl;
		}

		return SpectralBlob::Zero();
	}

	ParametricBlob parametric;
	mUpsampler->prepare(&value[0], &parametric[0], 1);

	return SpectralUpsampler::compute(parametric, ctx.WavelengthNM);
}

Vector2i NonParametricImageNode::queryRecommendedSize() const
{
	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (spec) {
		return Vector2i(spec->width, spec->height);
	} else {
		return Vector2i(1, 1);
	}
}

std::string NonParametricImageNode::dumpInformation() const
{
	std::stringstream stream;
	stream << mFilename << " [NonParam]";
	if (mIsPtex)
		stream << "[Ptex]";
	return stream.str();
}

//// Scalar
ScalarImageNode::ScalarImageNode(OIIO::TextureSystem* tsys,
								 const OIIO::TextureOpt& options,
								 const std::string& filename)
	: FloatScalarNode()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
	, mIsPtex(false)
	, mErrorIdenticator(false)
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
		const OIIO::ImageIOParameter* ptex = spec->find_attribute("ptex:meshType", OIIO::TypeDesc::STRING);
		if (ptex && ptex->type() == OIIO::TypeDesc::STRING) {
			mIsPtex = true;

			std::string type = spec->get_string_attribute("ptex::meshType");
			if (type != "triangle")
				PR_LOG(L_WARNING) << mFilename << ": Ptex support is only available for triangles!" << std::endl;
		}
	}
}

float ScalarImageNode::eval(const ShadingContext& ctx) const
{
	PR_ASSERT(mTextureSystem, "Given texture system has to be valid");

	OIIO::TextureOpt ops = mTextureOptions;

	if (mIsPtex)
		ops.subimage = static_cast<int>(ctx.Face);

	float value;
	if (!mTextureSystem->texture(mFilename, ops,
								 ctx.UV(0), 1 - ctx.UV(1),
								 0.0f, 0.0f, 0.0f, 0.0f,
								 /*ctx.dUV(0), ctx.dUV(1), ctx.dUV(0), ctx.dUV(1)*/
								 1, &value)) {

		if (!mErrorIdenticator.exchange(true)) {
			const std::string err = mTextureSystem->geterror();
			PR_LOG(L_ERROR) << "Could not lookup texture [" << mFilename << "]: " << err << std::endl;
		}

		return 0.0f;
	}

	return value;
}

/*Vector2i ScalarImageNode::queryRecommendedSize() const
{
	const OIIO::ImageSpec* spec = mTextureSystem->imagespec(mFilename);
	if (spec) {
		return Vector2i(spec->width, spec->height);
	} else {
		return Vector2i(1, 1);
	}
}*/

std::string ScalarImageNode::dumpInformation() const
{
	std::stringstream stream;
	stream << mFilename << " [Scalar]";
	if (mIsPtex)
		stream << "[Ptex]";
	return stream.str();
}
} // namespace PR
