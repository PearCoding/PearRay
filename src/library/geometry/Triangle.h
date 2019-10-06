#pragma once

#include "BoundingBox.h"
#include "math/SIMD.h"

#include <utility>

#define PR_TRIANGLE_INTERSECT_EPSILON (1e-4f)

namespace PR {
class PR_LIB_INLINE Triangle {
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
		using namespace simdpp;
		using ibool = typename VectorTemplate<T>::bool_t;

		const Vector3t<T> e12 = p2 - p1;
		const Vector3t<T> e13 = p3 - p1;

		const Vector3t<T> q = in.Direction.cross(e13);
		const T a			= q.dot(e12);

		const T f			= 1.0f / a;
		const Vector3t<T> s = in.Origin - p1;
		uv(0)				= f * s.dot(q);

		const ibool uoutside = (uv(0) < 0) | (uv(0) > 1);

		const Vector3t<T> r = s.cross(e12);
		uv(1)				= f * in.Direction.dot(r);

		const ibool voutside = (uv(1) < 0) | (uv(0) + uv(1) > 1);

		t = f * r.dot(e13);
		return b_and(b_and(b_not(uoutside), b_not(voutside)),
					 (t >= PR_TRIANGLE_INTERSECT_EPSILON));
	}
};
} // namespace PR
