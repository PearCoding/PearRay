#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB Fresnel {
	PR_CLASS_NON_CONSTRUCTABLE(Fresnel);

public:
	static inline float dielectric(float dot, float n1, float n2)
	{
		// Snells Law
		const float eta = n1 / n2; // Actually inverse eta
		const float z   = 1 - eta * eta * (1 - dot * dot);

		if (z <= PR_EPSILON)
			return 1;

		const float dotT = std::sqrt(z);
		const float para = (n1 * dot - n2 * dotT) / (n1 * dot + n2 * dotT);
		const float perp = (n1 * dotT - n2 * dot) / (n1 * dotT + n2 * dot);

		const float R = (para * para + perp * perp) / 2;
		return std::min(std::max(R, 0.0f), 1.0f);
	}

	static inline float conductor(float dot, float n, float k)
	{
		if (dot <= PR_EPSILON)
			return 1;

		const float dot2 = dot * dot;
		const float f	= (n * n + k * k);
		const float d1   = f * dot2;
		const float d2   = 2 * n * dot;

		const float para = (d1 - d2) / (d1 + d2);
		const float perp = (f - d2 + dot2) / (f + d2 + dot2);

		const float R = (para * para + perp * perp) / 2;
		return std::min(std::max(R, 0.0f), 1.0f);
	}

	static inline float schlick(float dot, float n1, float n2)
	{
		const float c = (n1 - n2) / (n1 + n2);
		return schlick(dot, c * c);
	}

	static inline float schlick(float dot, float f0)
	{
		return f0 + (1 - f0) * schlick_term(dot);
	}

	static inline float schlick_term(float dot)
	{
		const float t = 1 - dot;

		return (t * t) * (t * t) * t;
	}
};
}
