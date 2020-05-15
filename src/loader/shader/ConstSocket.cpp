#include "ConstSocket.h"
#include "spectral/SpectralUpsampler.h"

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

ConstSpectralShadingSocket::ConstSpectralShadingSocket(const ParametricBlob& f)
	: FloatSpectralShadingSocket()
	, mValue(f)
{
}

// TODO: Better way -> Hero Wavelength!
SpectralBlob ConstSpectralShadingSocket::eval(const ShadingPoint& ctx) const
{
	return SpectralUpsampler::compute(mValue, ctx.Ray.WavelengthNM);
}

float ConstSpectralShadingSocket::relativeLuminance(const ShadingPoint&) const
{
	// TODO?
	return mValue(0);
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

ConstSpectralMapSocket::ConstSpectralMapSocket(const ParametricBlob& f)
	: FloatSpectralMapSocket()
	, mValue(f)
{
}

SpectralBlob ConstSpectralMapSocket::eval(const MapSocketCoord& ctx) const
{
	return SpectralUpsampler::compute(mValue, ctx.WavelengthNM);
}

float ConstSpectralMapSocket::relativeLuminance(const MapSocketCoord&) const
{
	// TODO
	return mValue(0);
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
