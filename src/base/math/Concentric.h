#pragma once

#include "PR_Config.h"

namespace PR {
// Numerous Concentric disk from and to square mappings based on Shirley and Chiu, 1997
namespace Concentric {
/// Calculates radius [0, 1) phi[0, 2PI) coordinates from square coordinates
Vector2f square2rphi(const Vector2f& sq)
{
	Vector2f off = 2 * sq - Vector2f(1, 1);
	if (off(0) * off(0) > off(1) * off(1))
		return Vector2f(off(0), PR_PI * off(1) / (4 * off(0)));
	else
		return Vector2f(off(1), PR_PI / 2 - PR_PI * off(0) / (4 * off(1)));
}

/// Calculates square coordinates from radius [0, 1) phi[0, 2PI) coordinates
Vector2f rphi2square(const Vector2f& rphi)
{
	return Vector2f(0, 0); // TODO
}

/// Calculates disk UV coordinates from square coordinates
Vector2f square2disc(const Vector2f& sq)
{
	const Vector2f rphi = square2rphi(sq);
	return rphi(0) * Vector2f(std::cos(rphi(1)), std::sin(rphi(1)));
}

/// Calculates square coordinates from disc UV coordinates
Vector2f disc2square(const Vector2f& ds)
{
	const auto sign = [](float f) { return f < 0 ? -1 : 1; };
	const float N	= ds.norm();

	if (ds(0) * ds(0) > ds(1) * ds(1))
		return N * Vector2f(sign(ds(0)), 4 * PR_INV_PI * std::atan2(ds(1), std::abs(ds(0))));
	else
		return N * Vector2f(4 * PR_INV_PI * std::atan2(ds(0), std::abs(ds(1)) + 0.0001f), sign(ds(1)));
}
} // namespace Concentric
} // namespace PR
