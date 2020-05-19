#include "IlluminantSocket.h"

#include <sstream>

namespace PR {
IlluminantSpectralMapSocket::IlluminantSpectralMapSocket(const float* data, size_t sample_count, float wavelength_start, float wavelength_end)
	: FloatSpectralMapSocket()
	, mData(data)
	, mSampleCount(sample_count)
	, mWavelengthStart(wavelength_start)
	, mWavelengthEnd(wavelength_end)
	, mWavelengthDelta((wavelength_end - wavelength_start) / (sample_count - 1))
{
}

SpectralBlob IlluminantSpectralMapSocket::eval(const MapSocketCoord& ctx) const
{
	SpectralBlob blob;

	PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
	for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k) {
		const float f	= std::max(0.0f, (ctx.WavelengthNM[k] - mWavelengthStart) / mWavelengthDelta);
		const int index = std::min<int>(mSampleCount - 2, f);
		const float t	= f - index;

		blob[k] = mData[index] * (1 - t) + mData[index + 1] * t;
	}
	return blob;
}

Vector2i IlluminantSpectralMapSocket::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string IlluminantSpectralMapSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << "Illuminant (" << mSampleCount << ", [" << mWavelengthStart << ", " << mWavelengthEnd << "])";
	return sstream.str();
}

/////////////////////////////////////
#include "IlluminantData.inl"

#define _ILLUMINANT_DEC(Prefix, Samples, Start, End)                            \
	Prefix##Illuminant::Prefix##Illuminant(float power)                         \
		: IlluminantSpectralMapSocket(Prefix##_data, Samples, Start, End)       \
		, mPower(power)                                                         \
	{                                                                           \
	}                                                                           \
	SpectralBlob Prefix##Illuminant::eval(const MapSocketCoord& ctx) const      \
	{                                                                           \
		return IlluminantSpectralMapSocket::eval(ctx) * mPower;                 \
	}                                                                           \
	std::string Prefix##Illuminant::dumpInformation() const                     \
	{                                                                           \
		std::stringstream sstream;                                              \
		sstream << "Illuminant " PR_STRINGIFY(Prefix) " (P: " << mPower << ")"; \
		return sstream.str();                                                   \
	}

#define _ILLUMINANT_DEC_C(Prefix) _ILLUMINANT_DEC(Prefix, CIE_SampleCount, CIE_WavelengthStart, CIE_WavelengthEnd)
#define _ILLUMINANT_DEC_F(Prefix) _ILLUMINANT_DEC(Prefix, CIE_F_SampleCount, CIE_F_WavelengthStart, CIE_F_WavelengthEnd)

_ILLUMINANT_DEC_C(D65)
_ILLUMINANT_DEC_C(D50)
_ILLUMINANT_DEC_C(D55)
_ILLUMINANT_DEC_C(D75)
_ILLUMINANT_DEC_C(A)
_ILLUMINANT_DEC_C(C)

_ILLUMINANT_DEC_F(F1)
_ILLUMINANT_DEC_F(F2)
_ILLUMINANT_DEC_F(F3)
_ILLUMINANT_DEC_F(F4)
_ILLUMINANT_DEC_F(F5)
_ILLUMINANT_DEC_F(F6)
_ILLUMINANT_DEC_F(F7)
_ILLUMINANT_DEC_F(F8)
_ILLUMINANT_DEC_F(F9)
_ILLUMINANT_DEC_F(F10)
_ILLUMINANT_DEC_F(F11)
_ILLUMINANT_DEC_F(F12)
} // namespace PR
