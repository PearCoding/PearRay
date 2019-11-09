#include "ConstShadingSocket.h"

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

float ConstSpectralShadingSocket::eval(const ShadingPoint& ctx) const
{
	return mValue[ctx.Ray.WavelengthIndex];
}

float ConstSpectralShadingSocket::relativeLuminance(const ShadingPoint&) const
{
	return mValue.relativeLuminance();
}

std::string ConstSpectralShadingSocket::dumpInformation() const
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

float ConstVectorShadingSocket::eval(uint32 channel, const ShadingPoint&) const
{
	return mValue(channel);
}

std::string ConstVectorShadingSocket::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}
} // namespace PR
