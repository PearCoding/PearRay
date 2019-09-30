#pragma once

#include "ShadingPoint.h"

namespace PR {
template <typename T>
class PR_LIB_INLINE ScalarShadingSocket {
public:
	ScalarShadingSocket() = default;
	virtual ~ScalarShadingSocket() = default;

	virtual T eval(const ShadingPoint& ctx) const = 0;
};

using FloatScalarShadingSocket = ScalarShadingSocket<float>;

///////////////////
template <typename T>
class PR_LIB_INLINE SpectralShadingSocket {
public:
	SpectralShadingSocket() = default;
	virtual ~SpectralShadingSocket() = default;

	virtual T eval(const ShadingPoint& ctx) const = 0;
};

using FloatSpectralShadingSocket = SpectralShadingSocket<float>;

///////////////////
template <typename T>
class PR_LIB_INLINE VectorShadingSocket {
public:
	VectorShadingSocket() = default;
	virtual ~VectorShadingSocket() = default;

	virtual T eval(uint32 channel, const ShadingPoint& ctx) const = 0;
};

using FloatVectorShadingSocket = VectorShadingSocket<float>;
} // namespace PR
