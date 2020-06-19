#pragma once

#include "BoundingBox.h"

#include <utility>

namespace PR {
class PR_LIB_CORE Quad {
	PR_CLASS_NON_CONSTRUCTABLE(Quad);

public:
	inline static BoundingBox getBoundingBox(
		const Vector3f& p1, const Vector3f& p2, const Vector3f& p3, const Vector3f& p4)
	{
		BoundingBox box(p1, p2);
		box.combine(p3);
		box.combine(p4);
		//box.inflate(0.0001f);

		return box;
	}

	template<typename T>
	inline static T interpolate(const T& v0, const T& v1, const T& v2, const T& v3,
								const Vector2f& uv)
	{
		return v0 * (1 - uv(0)) * (1 - uv(1))
			   + v1 * uv(0) * (1 - uv(1))
			   + v2 * (1 - uv(0)) * uv(1)
			   + v3 * uv(0) * uv(1);
	}

	inline static float surfaceArea(const Vector3f& p1,
								const Vector3f& p2,
								const Vector3f& p3,
								const Vector3f& p4)
	{
		// Assuming convex quad
		return 0.5f * (p3 - p1).cross(p4 - p2).norm();
	}
};
} // namespace PR
