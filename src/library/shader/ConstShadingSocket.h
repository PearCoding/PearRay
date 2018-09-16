#pragma once

#include "shader/ShadingSocket.h"

namespace PR {
class PR_LIB ConstScalarShadingSocket : public FloatShadingSocket {
public:
	explicit ConstScalarShadingSocket(float f);
	vfloat eval(size_t channel, const ShadingPoint& ctx) const override;

private:
	float mValue;
};
} // namespace PR
