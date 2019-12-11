#include "TriangleFilter.h"

namespace PR {
float TriangleFilter::evalWeight(float x, float y) const
{
	float fx = std::max(0.0f, mRadius - std::abs(x)) / (2 * mRadius + 1);
	float fy = std::max(0.0f, mRadius - std::abs(y)) / (2 * mRadius + 1);

	return fx * fy;
}
} // namespace PR