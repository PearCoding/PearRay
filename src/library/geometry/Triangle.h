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
		const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3)
	{
		BoundingBox box(p1, p2);
		box.combine(p3);
		//box.inflate(0.0001f);

		return box;
	}

	template <typename T>
	inline static T surfaceArea(
		const T& p1x, const T& p1y, const T& p1z,
		const T& p2x, const T& p2y, const T& p2z,
		const T& p3x, const T& p3y, const T& p3z)
	{
		using namespace simdpp;

		const T e12x = p2x - p1x;
		const T e12y = p2y - p1y;
		const T e12z = p2z - p1z;

		const T e13x = p3x - p1x;
		const T e13y = p3y - p1y;
		const T e13z = p3z - p1z;

		T c1, c2, c3;
		crossV(e12x, e12y, e12z, e13x, e13y, e13z, c1, c2, c3);
		return 0.5f * sqrt(dotV(c1, c2, c3, c1, c2, c3));
	}

	// SIMD
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	template <typename T, typename R>
	inline static typename RayTemplate<R>::bool_t intersect(const R& in,
															const T& p1x, const T& p1y, const T& p1z,
															const T& p2x, const T& p2y, const T& p2z,
															const T& p3x, const T& p3y, const T& p3z,
															typename RayTemplate<R>::float_t& u,
															typename RayTemplate<R>::float_t& v,
															typename RayTemplate<R>::float_t& t)
	{
		using namespace simdpp;
		using ifloat = typename RayTemplate<R>::float_t;
		using ibool  = typename RayTemplate<R>::bool_t;

		const T e12x = p2x - p1x;
		const T e12y = p2y - p1y;
		const T e12z = p2z - p1z;

		const T e13x = p3x - p1x;
		const T e13y = p3y - p1y;
		const T e13z = p3z - p1z;

		ifloat q1, q2, q3;
		crossV(in.Direction[0], in.Direction[1], in.Direction[2],
			   e13x, e13y, e13z,
			   q1, q2, q3);

		const ifloat a = dotV(q1, q2, q3, e12x, e12y, e12z);

		const ifloat f  = 1.0f / a;
		const ifloat s1 = in.Origin[0] - p1x;
		const ifloat s2 = in.Origin[1] - p1y;
		const ifloat s3 = in.Origin[2] - p1z;

		u = f * dotV(s1, s2, s3, q1, q2, q3);

		const ibool uoutside = (u < 0) | (u > 1);

		ifloat r1, r2, r3;
		crossV(s1, s2, s3,
			   e12x, e12y, e12z,
			   r1, r2, r3);

		v = f * dotV(in.Direction[0], in.Direction[1], in.Direction[2], r1, r2, r3);

		const ibool voutside = (v < 0) | (u + v > 1);

		t = f * dotV(r1, r2, r3, e13x, e13y, e13z);
		return b_and(b_and(b_not(uoutside), b_not(voutside)), (t >= PR_TRIANGLE_INTERSECT_EPSILON));
	}
};
} // namespace PR
