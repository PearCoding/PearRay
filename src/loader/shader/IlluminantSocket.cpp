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

constexpr size_t D65_SampleCount	   = 107;
constexpr float D65_WavelengthStart	   = 300.0f;
constexpr float D65_WavelengthEnd	   = 830.0f;
static float D65_data[D65_SampleCount] = {
	0.000341f, 0.016643f, 0.032945f, 0.117652f,
	0.20236f, 0.286447f, 0.370535f, 0.385011f,
	0.399488f, 0.424302f, 0.449117f, 0.45775f,
	0.466383f, 0.493637f, 0.520891f, 0.510323f,
	0.499755f, 0.523118f, 0.546482f, 0.687015f,
	0.827549f, 0.871204f, 0.91486f, 0.924589f,
	0.934318f, 0.90057f, 0.866823f, 0.957736f,
	1.04865f, 1.10936f, 1.17008f, 1.1741f,
	1.17812f, 1.16336f, 1.14861f, 1.15392f,
	1.15923f, 1.12367f, 1.08811f, 1.09082f,
	1.09354f, 1.08578f, 1.07802f, 1.06296f,
	1.0479f, 1.06239f, 1.07689f, 1.06047f,
	1.04405f, 1.04225f, 1.04046f, 1.02023f,
	1.0f, 0.981671f, 0.963342f, 0.960611f,
	0.95788f, 0.922368f, 0.886856f, 0.893459f,
	0.900062f, 0.898026f, 0.895991f, 0.886489f,
	0.876987f, 0.854936f, 0.832886f, 0.834939f,
	0.836992f, 0.81863f, 0.800268f, 0.801207f,
	0.802146f, 0.812462f, 0.822778f, 0.80281f,
	0.782842f, 0.740027f, 0.697213f, 0.706652f,
	0.716091f, 0.72979f, 0.74349f, 0.679765f,
	0.61604f, 0.657448f, 0.698856f, 0.724863f,
	0.75087f, 0.693398f, 0.635927f, 0.550054f,
	0.464182f, 0.566118f, 0.668054f, 0.650941f,
	0.633828f, 0.638434f, 0.64304f, 0.618779f,
	0.594519f, 0.557054f, 0.51959f, 0.546998f,
	0.574406f, 0.588765f, 0.603125f
};

D65Illuminant::D65Illuminant(float power)
	: IlluminantSpectralMapSocket(D65_data, D65_SampleCount, D65_WavelengthStart, D65_WavelengthEnd)
	, mPower(power)
{
}

SpectralBlob D65Illuminant::eval(const MapSocketCoord& ctx) const
{
	return IlluminantSpectralMapSocket::eval(ctx) * mPower;
}

std::string D65Illuminant::dumpInformation() const
{
	std::stringstream sstream;
	sstream << "Illuminant D65 (P: " << mPower << ")";
	return sstream.str();
}
} // namespace PR
