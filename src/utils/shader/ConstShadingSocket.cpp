#include "ConstShadingSocket.h"

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
} // namespace PR
