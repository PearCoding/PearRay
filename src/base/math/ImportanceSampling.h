#pragma once

#include "PR_Config.h"

namespace PR {
namespace IS {

inline float balance_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	return n1 * pdf1 / (n1 * pdf1 + n2 * pdf2);
}

template <typename Derived1, typename Derived2>
inline float balance_term(uint32 n1, const Eigen::ArrayBase<Derived1>& pdf1, uint32 n2, const Eigen::ArrayBase<Derived2>& pdf2)
{
	return n1 * pdf1 / (n1 * pdf1 + n2 * pdf2);
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	const float t1 = n1 * pdf1, t2 = n2 * pdf2;
	return (t1 * t1) / (t1 * t1 + t2 * t2);
}

template <typename Derived1, typename Derived2>
inline float power_term(uint32 n1, const Eigen::ArrayBase<Derived1>& pdf1, uint32 n2, const Eigen::ArrayBase<Derived2>& pdf2)
{
	const auto t1 = n1 * pdf1, t2 = n2 * pdf2;
	return (t1.squared()) / (t1.squared() + t2.squared());
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2, float beta)
{
	const float t1 = std::pow(n1 * pdf1, beta), t2 = std::pow(n2 * pdf2, beta);
	return (t1) / (t1 + t2);
}

template <typename Derived1, typename Derived2>
inline float power_term(uint32 n1, const Eigen::ArrayBase<Derived1>& pdf1, uint32 n2, const Eigen::ArrayBase<Derived2>& pdf2, float beta)
{
	const auto t1 = std::pow(n1 * pdf1, beta), t2 = std::pow(n2 * pdf2, beta);
	return (t1) / (t1 + t2);
}

// Casts
/* dw/dA = cos(N.L)/r^2
 * dw    = dA * cos(N.L)/r^2
 * dA    = dw * r^2/cos(N.L)
 */
inline float toArea(float pdf_solidangle, float dist_sqr, float abs_cosine)
{
	PR_ASSERT(abs_cosine >= 0, "absolute_cosine has to be greater or equal 0");
	return pdf_solidangle * abs_cosine / dist_sqr;
}

inline float toSolidAngle(float pdf_area, float dist_sqr, float abs_cosine)
{
	PR_ASSERT(abs_cosine >= 0, "absolute_cosine has to be greater or equal 0");
	return pdf_area * dist_sqr / abs_cosine;
}
} // namespace MSI
} // namespace PR
