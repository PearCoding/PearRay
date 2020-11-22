#pragma once

#include "PR_Config.h"

namespace PR {
inline float correctShadingNormalForLight(const Vector3f& V, const Vector3f& L, const Vector3f& Ns, const Vector3f& Ng)
{
	float numerator	  = std::abs(V.dot(Ns) * L.dot(Ng));
	float denumerator = std::abs(V.dot(Ng) * L.dot(Ns));
	return denumerator > PR_EPSILON ? numerator / denumerator : 0.0f;
}

static inline float culling(float cos)
{
#ifdef PR_NO_CULLING
	return std::abs(cos);
#else
	return std::max(0.0f, cos);
#endif
}
} // namespace PR