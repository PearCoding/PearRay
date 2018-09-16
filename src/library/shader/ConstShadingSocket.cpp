#include "ConstShadingSocket.h"

namespace PR {
ConstScalarShadingSocket::ConstScalarShadingSocket(float f)
	: FloatShadingSocket()
	, mValue(f)
{
}

vfloat ConstScalarShadingSocket::eval(size_t channel, const ShadingPoint& ctx) const
{
	return simdpp::make_float(mValue);
}
} // namespace PR
