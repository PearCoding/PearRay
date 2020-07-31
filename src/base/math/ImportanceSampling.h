#pragma once

#include "PR_Config.h"

namespace PR {
namespace IS {

inline float balance_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	return 1 / (1 + (n2 * pdf2) / (n1 * pdf1));
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	const float t = (n2 * pdf2) / (n1 * pdf1);
	return 1 / (1 + t * t);
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2, float beta)
{
	const float t = (n2 * pdf2) / (n1 * pdf1);
	return 1 / (1 + std::pow(t, beta));
}

// Casts
/* dw/dA = cos(N.L)/r^2
 * dw    = dA * cos(N.L)/r^2
 * dA    = dw * r^2/cos(N.L)
 */
inline float toArea(float pdf_solidangle, float dist_sqr, float abs_cosine)
{
	PR_ASSERT(abs_cosine >= 0.0f, "absolute_cosine has to be greater or equal 0");
	return pdf_solidangle * abs_cosine / dist_sqr;
}

inline float toSolidAngle(float pdf_area, float dist_sqr, float abs_cosine)
{
	PR_ASSERT(abs_cosine >= 0.0f, "absolute_cosine has to be greater or equal 0");
	return pdf_area * dist_sqr / abs_cosine;
}
} // namespace IS
} // namespace PR
