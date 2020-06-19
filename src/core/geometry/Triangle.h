#pragma once

#include "BoundingBox.h"

namespace PR {
class PR_LIB_CORE Triangle {
	PR_CLASS_NON_CONSTRUCTABLE(Triangle);

public:
	inline static BoundingBox getBoundingBox(
		const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
	{
		BoundingBox box(p1, p2);
		box.combine(p3);
		//box.inflate(0.0001f);

		return box;
	}

	template <typename T>
	inline static T interpolate(const T& v0, const T& v1, const T& v2,
								const Vector2f& uv)
	{
		return v1 * uv(0) + v2 * uv(1) + v0 * (1 - uv(0) - uv(1));
	}

	inline static float surfaceArea(const Vector3f& p1,
									const Vector3f& p2,
									const Vector3f& p3)
	{
		const auto e12 = p2 - p1;
		const auto e13 = p3 - p1;
		return 0.5f * e12.cross(e13).norm();
	}

	// Based on "A Low-Distortion Map Between Triangle and Square" by Eric Heitz
	// and https://pharr.org/matt/blog/2019/03/13/triangle-sampling-1.5.html
	// Expects uniform values [0,1)^2 and maps them to the barycentric domain
	inline static Vector2f sample(const Vector2f& uniform)
	{
		if (uniform(1) > uniform(0)) {
			float x = uniform(0) / 2;
			return Vector2f(x, uniform(1) - x);
		} else {
			float y = uniform(1) / 2;
			return Vector2f(uniform(0) - y, y);
		}
	}
};
} // namespace PR
