#pragma once

#include "ShadingPoint.h"

namespace PR {
template <typename T>
class PR_LIB_INLINE ShadingSocket {
public:
	ShadingSocket<T>() = default;
	virtual ~ShadingSocket<T>() {}

	virtual T eval(size_t channel, const ShadingPoint& ctx) const = 0;
};

typedef ShadingSocket<vfloat> FloatShadingSocket;
} // namespace PR
