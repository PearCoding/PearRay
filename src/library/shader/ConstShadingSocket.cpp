#include "ConstShadingSocket.h"

namespace PR {
ConstScalarShadingSocket::ConstScalarShadingSocket(float f)
	: FloatScalarShadingSocket()
	, mValue(f)
{
}

vfloat ConstScalarShadingSocket::eval(const ShadingPoint&) const
{
	return simdpp::make_float(mValue);
}

/////////////////////////////////////

ConstSpectralShadingSocket::ConstSpectralShadingSocket(const Spectrum& f)
	: FloatSpectralShadingSocket()
	, mValue(f)
{
}

vfloat ConstSpectralShadingSocket::eval(const ShadingPoint& ctx) const
{
	return simdpp::make_float(mValue[ctx.WavelengthIndex]);
}

/////////////////////////////////////

ConstVectorShadingSocket::ConstVectorShadingSocket(const Eigen::Vector3f& f)
	: FloatVectorShadingSocket()
	, mValue(f(0),f(1),f(2))
{
}

vfloat ConstVectorShadingSocket::eval(uint32 channel, const ShadingPoint&) const
{
	return simdpp::make_float(mValue(channel));
}
} // namespace PR
