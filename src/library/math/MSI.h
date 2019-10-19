#pragma once

#include "spectral/Spectrum.h"

namespace PR {
namespace MSI {

inline float balance_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	return n1 * pdf1 / (n1 * pdf1 + n2 * pdf2);
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2)
{
	const float t1 = n1 * pdf1, t2 = n2 * pdf2;
	return (t1 * t1) / (t1 * t1 + t2 * t2);
}

inline float power_term(uint32 n1, float pdf1, uint32 n2, float pdf2, float beta)
{
	const float t1 = std::pow(n1 * pdf1, beta), t2 = std::pow(n2 * pdf2, beta);
	return (t1) / (t1 + t2);
}

inline float power(float& out_pdf, float in_pdf)
{
	PR_ASSERT(out_pdf >= 0, "outgoing pdf has to greater or equal 0");
	PR_ASSERT(in_pdf >= 0, "ingoing pdf has to greater or equal 0");

	float w;
	if (out_pdf < in_pdf) {
		const float r = out_pdf / in_pdf;
		w			  = 1 / (1 + r * r);
	} else if (in_pdf < out_pdf) {
		const float r = in_pdf / out_pdf;
		w			  = 1 - 1 / (1 + r * r);
	} else {
		w = 0.5f;
	}

	out_pdf += in_pdf;

	return w;
}

template <typename T, typename = Lazy::enable_if_slo_t<T, T, void>>
inline void power(Spectrum& out_weight, float& out_pdf, const T& in_weight, float in_pdf)
{
	float w = power(out_pdf, in_pdf);
	out_weight.lerp(in_weight, w);
}

inline float power(float& out_pdf, float in_pdf, float beta)
{
	PR_ASSERT(out_pdf >= 0, "outgoing pdf has to greater or equal 0");
	PR_ASSERT(in_pdf >= 0, "ingoing pdf has to greater or equal 0");

	float w;
	if (out_pdf < in_pdf) {
		const float r = out_pdf / in_pdf;
		w			  = 1 / (1 + std::pow(r, beta));
	} else if (in_pdf < out_pdf) {
		const float r = in_pdf / out_pdf;
		w			  = 1 - 1 / (1 + std::pow(r, beta));
	} else {
		w = 0.5f;
	}

	out_pdf += in_pdf;

	return w;
}

template <typename T, typename = Lazy::enable_if_slo_t<T, T, void>>
inline void power(Spectrum& out_weight, float& out_pdf, const T& in_weight, float in_pdf, float beta)
{
	float w = power(out_pdf, in_pdf, beta);
	out_weight.lerp(in_weight, w);
}

inline float balance(float& out_pdf, float in_pdf)
{
	PR_ASSERT(out_pdf >= 0, "outgoing pdf has to greater or equal 0");
	PR_ASSERT(in_pdf >= 0, "ingoing pdf has to greater or equal 0");

	float w;
	if (out_pdf < in_pdf)
		w = 1 / (1 + out_pdf / in_pdf);
	else if (in_pdf < out_pdf)
		w = 1 - 1 / (1 + in_pdf / out_pdf);
	else
		w = 0.5f;

	out_pdf += in_pdf;

	return w;
}

template <typename T, typename = Lazy::enable_if_slo_t<T, T, void>>
inline void balance(Spectrum& out_weight, float& out_pdf, const T& in_weight, float in_pdf)
{
	float w = balance(out_pdf, in_pdf);
	out_weight.lerp(in_weight, w);
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
