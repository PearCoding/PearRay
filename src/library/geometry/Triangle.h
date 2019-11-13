#pragma once

#include "BoundingBox.h"
#include "TriangleIntersection.h"

#include <utility>

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
	inline static T surfaceArea(const Vector3t<T>& p1,
								const Vector3t<T>& p2,
								const Vector3t<T>& p3)
	{
		using namespace simdpp;

		const auto e12 = p2 - p1;
		const auto e13 = p3 - p1;
		return 0.5f * e12.cross(e13).norm();
	}

	// SIMD
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	template <typename T>
	inline static typename VectorTemplate<T>::bool_t intersect(
		const RayPackageBase<T>& in,
		const Vector3t<T>& p1,
		const Vector3t<T>& p2,
		const Vector3t<T>& p3,
		Vector2t<T>& uv,
		T& t)
	{
		return TriangleIntersection::intersectMT(in, p1, p2, p3, uv, t);
	}
};
} // namespace PR
