#include "geometry/Triangle.h"
#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Triangle)
PR_TEST("Intersects CCW")
{
	RayPackage in(Vector3fv(simdpp::make_float(0.5, 0.25, 0, 0.6),
							simdpp::make_float(0.5, 0.75, 0, 0.6),
							simdpp::make_float(-1, -1, -1, -1)),
				  Vector3fv(simdpp::make_float(0),
							simdpp::make_float(0),
							simdpp::make_float(1)));

	in.setupInverse();

	const Vector3fv p0 = Vector3fv(vfloat(0), vfloat(0), vfloat(0));
	const Vector3fv p1 = Vector3fv(vfloat(1), vfloat(0), vfloat(0));
	const Vector3fv p2 = Vector3fv(vfloat(0), vfloat(1), vfloat(0));

	Vector2fv uv;
	vfloat t;
	const bfloat res = Triangle::intersect(in,
										   p0, p1, p2,
										   uv, t);

	PR_CHECK_TRUE(any(res));
	PR_CHECK_FALSE(all(res));

	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("Intersects CW")
{
	RayPackage in(Vector3fv(simdpp::make_float(0.5, 0.25, 0, 0.6),
							simdpp::make_float(0.5, 0.75, 0, 0.6),
							simdpp::make_float(-1, -1, -1, -1)),
				  Vector3fv(simdpp::make_float(0),
							simdpp::make_float(0),
							simdpp::make_float(1)));

	in.setupInverse();

	const Vector3fv p0 = Vector3fv(vfloat(0), vfloat(0), vfloat(0));
	const Vector3fv p1 = Vector3fv(vfloat(0), vfloat(1), vfloat(0));
	const Vector3fv p2 = Vector3fv(vfloat(1), vfloat(0), vfloat(0));

	Vector2fv uv;
	vfloat t;
	const bfloat res = Triangle::intersect(in,
										   p0, p1, p2,
										   uv, t);

	PR_CHECK_TRUE(any(res));
	PR_CHECK_FALSE(all(res));

	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Triangle);
PRT_END_MAIN