#pragma once

#include "math/Vector.h"
#include "shader/ShadingSocket.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB_UTILS ConstScalarShadingSocket : public FloatScalarShadingSocket {
public:
	explicit ConstScalarShadingSocket(float f);
	vfloat eval(const ShadingPoint& ctx) const override;

private:
	float mValue;
};

class PR_LIB_UTILS ConstSpectralShadingSocket : public FloatSpectralShadingSocket {
public:
	explicit ConstSpectralShadingSocket(const Spectrum& f);
	vfloat eval(const ShadingPoint& ctx) const override;

private:
	Spectrum mValue;
};

class PR_LIB_UTILS ConstVectorShadingSocket : public FloatVectorShadingSocket {
public:
	explicit ConstVectorShadingSocket(const Eigen::Vector3f& f);
	vfloat eval(uint32 channel, const ShadingPoint& ctx) const override;

private:
	Vector3f mValue;
};
} // namespace PR
