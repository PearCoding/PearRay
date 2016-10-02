#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Fresnel
	{
		PR_CLASS_NON_COPYABLE(Fresnel);
	public:
		static inline float dielectric(float dot, float n1, float n2)
		{
			// Snells Law
			const float eta = n1 / n2;
			const float z = 1 - eta * eta * (1 - dot * dot);

			if (z < 0.0f)
				return 1;

			const float dotT = std::sqrt(z);
			const float para = (n1 * dot - n2 * dotT) / (n1 * dot + n2 * dotT);
			const float perp = (n1 * dotT - n2 * dot) / (n1 * dotT + n2 * dot);

			const float R = (para * para + perp * perp) / 2;
			return PM::pm_ClampT(R, 0.0f, 1.0f);
		}

		static inline float schlick(float f0, float NdotV)
		{
			return f0 + (1 - f0)*schlick_term(NdotV);
		}

		static inline float schlick_term(float NdotV)
		{
			const float t = 1 - NdotV;

			return t*t*t*t*t;
		}
	};
}