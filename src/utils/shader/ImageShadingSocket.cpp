#include "ImageShadingSocket.h"

#include "Logger.h"
#include "math/SIMD.h"

#define PR_OIIO_HAS_BATCH_SUPPORT (OIIO_VERSION_MAJOR >= 1 && OIIO_VERSION_MINOR >= 9)

namespace PR {
ImageShadingSocket::ImageShadingSocket(OIIO::TextureSystem* tsys,
									   const OIIO::TextureOpt& options,
									   const std::string& filename)
	: FloatShadingSocket()
	, mFilename(filename)
	, mTextureOptions(options)
	, mTextureSystem(tsys)
{
	PR_ASSERT(tsys, "Given texture system has to be valid");
	PR_ASSERT(!mFilename.empty(), "Given filename shouldn't be empty");
}

vfloat ImageShadingSocket::eval(size_t channel, const ShadingPoint& ctx) const
{
// Batch implementation is incomplete -> Disable it for now
//#if PR_OIIO_HAS_BATCH_SUPPORT
#if 0
	static_assert(OIIO::Tex::BatchWidth >= PR_SIMD_BANDWIDTH,
				  "OIIO texture batchwidth has to be greater or equal of internal simd bandwidth");

	OIIO::TextureOptBatch batchOp;
	batchOp.firstchannel = channel;
	batchOp.swrap		 = mTextureOptions.swrap;
	batchOp.twrap		 = mTextureOptions.twrap;
	batchOp.mipmode		 = mTextureOptions.mipmode;
	batchOp.interpmode   = mTextureOptions.interpmode;
	batchOp.anisotropic  = mTextureOptions.anisotropic;

	// TODO: blur
	// TODO: width

	OIIO::Tex::RunMask mask = (1 << PR_SIMD_BANDWIDTH) - 1;

	PR_SIMD_ALIGN float s[OIIO::Tex::BatchWidth];
	PR_SIMD_ALIGN float t[OIIO::Tex::BatchWidth];
	PR_SIMD_ALIGN float dsdx[OIIO::Tex::BatchWidth];
	PR_SIMD_ALIGN float dsdy[OIIO::Tex::BatchWidth];
	PR_SIMD_ALIGN float dtdx[OIIO::Tex::BatchWidth];
	PR_SIMD_ALIGN float dtdy[OIIO::Tex::BatchWidth];

	simdpp::store(s, ctx.UVW[0]);
	simdpp::store(t, 1 - ctx.UVW[1]);
	std::fill_n(dsdx, OIIO::Tex::BatchWidth, 0);
	std::fill_n(dsdy, OIIO::Tex::BatchWidth, 0);
	std::fill_n(dtdx, OIIO::Tex::BatchWidth, 0);
	std::fill_n(dtdy, OIIO::Tex::BatchWidth, 0);

	float* res = nullptr;
	if (!mTextureSystem->texture(mFilename, batchOp, mask,
								 s, t,
								 dsdx, dtdx, dsdy, dtdy,
								 1, res)) {
		std::string err = mTextureSystem->geterror();
		PR_LOG(L_ERROR) << "Couldn't lookup texture: " << err << std::endl;
	}

	vfloat l = simdpp::make_float(0);
	if (res) {
		l = simdpp::load(res);
	}
	return l;
#else
	OIIO::TextureOpt ops = mTextureOptions;
	ops.firstchannel	 = channel;

	PR_SIMD_ALIGN float res[PR_SIMD_BANDWIDTH];

	for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
		ops.subimage = extract(i, ctx.PrimID);
		if (!mTextureSystem->texture(mFilename, ops,
									 extract(i, ctx.UVW[0]), extract(i, ctx.UVW[1]),
									 0, 0, 0, 0,
									 1, &res[i])) {
			std::string err = mTextureSystem->geterror();
			PR_LOG(L_ERROR) << "Couldn't lookup texture: " << err << std::endl;
			return simdpp::make_float(0);
		}
	}

	return simdpp::load(res);
#endif
}
} // namespace PR
