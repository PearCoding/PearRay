#pragma once

#include "BoundingBox.h"

namespace PR {
class PR_LIB Triangle {
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

	template <typename T>
	inline static T surfaceArea(const Vector3t<T>& p1,
								const Vector3t<T>& p2,
								const Vector3t<T>& p3)
	{
		using namespace simdpp;

		const auto e12 = p2 - p1;
		const auto e13 = p3 - p1;
		return 0.5f * e12.cross(e13).norm();
	}
};
} // namespace PR
