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

	template<typename T>
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
	template<typename T>
	inline static bfloat intersect(const CollisionInput& in,
									const T& p1x, const T& p1y, const T& p1z,
									const T& p2x, const T& p2y, const T& p2z,
									const T& p3x, const T& p3y, const T& p3z,
									vfloat& u, vfloat& v, vfloat& t)
	{
		using namespace simdpp;

		const T e12x = p2x - p1x;
		const T e12y = p2y - p1y;
		const T e12z = p2z - p1z;

		const T e13x = p3x - p1x;
		const T e13y = p3y - p1y;
		const T e13z = p3z - p1z;

		vfloat q1, q2, q3;
		crossV(in.RayDirection[0], in.RayDirection[1], in.RayDirection[2],
			   e13x, e13y, e13z,
			   q1, q2, q3);

		const vfloat a = dotV(q1, q2, q3, e12x, e12y, e12z);

		const vfloat f  = 1.0f / a;
		const vfloat s1 = in.RayOrigin[0] - p1x;
		const vfloat s2 = in.RayOrigin[1] - p1y;
		const vfloat s3 = in.RayOrigin[2] - p1z;

		u = f * dotV(s1, s2, s3, q1, q2, q3);

		const bfloat uoutside = (u < 0) | (u > 1);

		vfloat r1, r2, r3;
		crossV(s1, s2, s3,
			   e12x, e12y, e12z,
			   r1, r2, r3);

		v = f * dotV(in.RayDirection[0], in.RayDirection[1], in.RayDirection[2], r1, r2, r3);

		const bfloat voutside = (v < 0) | (u + v > 1);

		t = f * dotV(r1, r2, r3, e13x, e13y, e13z);
		return (~uoutside) & (~voutside) & (t >= PR_TRIANGLE_INTERSECT_EPSILON);
	}
};
} // namespace PR
