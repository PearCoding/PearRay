#include "ConstSocket.h"

#include <sstream>

namespace PR {
ConstScalarShadingSocket::ConstScalarShadingSocket(float f)
	: FloatScalarShadingSocket()
	, mValue(f)
{
}

float ConstScalarShadingSocket::eval(const ShadingPoint&) const
{
	return mValue;
}

std::string ConstScalarShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ConstSpectralShadingSocket::ConstSpectralShadingSocket(const Spectrum& f)
	: FloatSpectralShadingSocket()
	, mValue(f)
{
}

// TODO: Better way -> Hero Wavelength!
SpectralBlob ConstSpectralShadingSocket::eval(const ShadingPoint& ctx) const
{
	PR_UNUSED(ctx);
	const uint32 index = 0;//ctx.Ray.WavelengthIndex;
	return SpectralBlob(mValue[index],
						index + 1 < mValue.samples() ? mValue[index + 1] : 0.0f,
						index + 2 < mValue.samples() ? mValue[index + 2] : 0.0f,
						index + 3 < mValue.samples() ? mValue[index + 3] : 0.0f);
}

float ConstSpectralShadingSocket::relativeLuminance(const ShadingPoint&) const
{
	return mValue.relativeLuminance();
}

Vector2i ConstSpectralShadingSocket::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string ConstSpectralShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ConstSpectralMapSocket::ConstSpectralMapSocket(const Spectrum& f)
	: FloatSpectralMapSocket()
	, mValue(f)
{
}

// TODO: Better way -> Hero Wavelength!
SpectralBlob ConstSpectralMapSocket::eval(const MapSocketCoord& ctx) const
{
	PR_UNUSED(ctx);
	const uint32 index = 0;//ctx.Ray.WavelengthIndex;
	return SpectralBlob(mValue[index],
						index + 1 < mValue.samples() ? mValue[index + 1] : 0.0f,
						index + 2 < mValue.samples() ? mValue[index + 2] : 0.0f,
						index + 3 < mValue.samples() ? mValue[index + 3] : 0.0f);
}

float ConstSpectralMapSocket::relativeLuminance(const MapSocketCoord&) const
{
	return mValue.relativeLuminance();
}

Vector2i ConstSpectralMapSocket::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string ConstSpectralMapSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ConstVectorShadingSocket::ConstVectorShadingSocket(const Vector3f& f)
	: FloatVectorShadingSocket()
	, mValue(f)
{
}

Vector3f ConstVectorShadingSocket::eval(const ShadingPoint&) const
{
	return mValue;
}

std::string ConstVectorShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}
} // namespace PR
