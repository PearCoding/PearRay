#pragma once

#include "PR_Config.h"

namespace PR {
namespace Normal {
/// Evaluate gaussian
inline float PR_LIB_BASE eval(float x, float mean, float stddev)
{
	constexpr float SQRT_2PI = 2.5066282746310f;
	const float k			 = (x - mean) / stddev;
	return std::exp(-k * k / 2) / (SQRT_2PI * stddev);
}

/// Inverse of the error function for -1 < x < 1
/// Based on https://stackoverflow.com/questions/27229371/inverse-error-function-in-c by njuffa
inline float PR_LIB_BASE erfinv(float x)
{
	const float t = std::log(std::max(std::fma(x, -x, 1.0f), std::numeric_limits<float>::min()));

	float p;
	if (std::abs(t) > 6.125f) {				 // maximum ulp error = 2.35793
		p = 3.03697567e-10f;				 //  0x1.4deb44p-32
		p = std::fma(p, t, 2.93243101e-8f);	 //  0x1.f7c9aep-26
		p = std::fma(p, t, 1.22150334e-6f);	 //  0x1.47e512p-20
		p = std::fma(p, t, 2.84108955e-5f);	 //  0x1.dca7dep-16
		p = std::fma(p, t, 3.93552968e-4f);	 //  0x1.9cab92p-12
		p = std::fma(p, t, 3.02698812e-3f);	 //  0x1.8cc0dep-9
		p = std::fma(p, t, 4.83185798e-3f);	 //  0x1.3ca920p-8
		p = std::fma(p, t, -2.64646143e-1f); // -0x1.0eff66p-2
		p = std::fma(p, t, 8.40016484e-1f);	 //  0x1.ae16a4p-1
	} else {								 // maximum ulp error = 2.35456
		p = 5.43877832e-9f;					 //  0x1.75c000p-28
		p = std::fma(p, t, 1.43286059e-7f);	 //  0x1.33b458p-23
		p = std::fma(p, t, 1.22775396e-6f);	 //  0x1.49929cp-20
		p = std::fma(p, t, 1.12962631e-7f);	 //  0x1.e52bbap-24
		p = std::fma(p, t, -5.61531961e-5f); // -0x1.d70c12p-15
		p = std::fma(p, t, -1.47697705e-4f); // -0x1.35be9ap-13
		p = std::fma(p, t, 2.31468701e-3f);	 //  0x1.2f6402p-9
		p = std::fma(p, t, 1.15392562e-2f);	 //  0x1.7a1e4cp-7
		p = std::fma(p, t, -2.32015476e-1f); // -0x1.db2aeep-3
		p = std::fma(p, t, 8.86226892e-1f);	 //  0x1.c5bf88p-1
	}
	return x * p;
}

/// Sample based on the gaussian
inline float PR_LIB_BASE sample(float u, float mean, float stddev)
{
	// Scale u from [0,1) to [EPS, 1-EPS)
	constexpr float EPS = 1e-5f;
	u					= (1 - 2 * EPS) * u + EPS;

	return mean + PR_SQRT2 * stddev * erfinv(2 * u - 1);
}

inline float pdf(float x, float mean, float stddev)
{
	return eval(x, mean, stddev);
}

/// Sample based on the truncated gaussian
/// This may have some problems! -> https://en.wikipedia.org/wiki/Truncated_normal_distribution
inline float PR_LIB_BASE sample(float u, float mean, float stddev, float a, float b)
{
	const float alpha = (a - mean) / stddev;
	const float beta  = (b - mean) / stddev;

	const float CDF_A = (1 + std::erf(alpha / PR_SQRT2)) / 2;
	const float CDF_B = (1 + std::erf(beta / PR_SQRT2)) / 2;

	return sample(CDF_A + u * (CDF_B - CDF_A), mean, stddev);
}

/// Truncated PDF
inline float pdf(float x, float mean, float stddev, float a, float b)
{
	const float alpha = (a - mean) / stddev;
	const float beta  = (b - mean) / stddev;

	const float CDF_A = (1 + std::erf(alpha / PR_SQRT2)) / 2;
	const float CDF_B = (1 + std::erf(beta / PR_SQRT2)) / 2;

	return pdf(x, mean, stddev) / (CDF_B - CDF_A);
}
} // namespace Normal
} // namespace PR