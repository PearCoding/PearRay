#include "geometry/Triangle.h"
#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

const Vector3f P0 = Vector3f(0, 0, 0);
const Vector3f P1 = Vector3f(1, 0, 0);
const Vector3f P2 = Vector3f(0, 1, 0);

const Vector3f D	= Vector3f(0, 0, 1);
const Vector3f P[4] = { Vector3f(0.5f, 0.5f, -1),
						Vector3f(0.25f, 0.75f, -1),
						Vector3f(0, 0, -1),
						Vector3f(0.6f, 0.6f, -1) };

const bool Res[4] = { true, true, true, false };

static RayPackage makeRayPackage()
{
	return RayPackage(Vector3fv(simdpp::make_float(P[0][0], P[1][0], P[2][0], P[3][0]),
								simdpp::make_float(P[0][1], P[1][1], P[2][1], P[3][1]),
								simdpp::make_float(P[0][2], P[1][2], P[2][2], P[3][2])),
					  promote(D));
}

static Ray makeRay(size_t i)
{
	return Ray(P[i], D);
}

PR_BEGIN_TESTCASE(Triangle)
PR_TEST("MT Intersects CCW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectMT(makeRayPackage(),
														 promote(P0), promote(P1), promote(P2),
														 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("MT Intersects CW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectMT(makeRayPackage(),
														 promote(P0), promote(P2), promote(P1),
														 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("MT Intersects CCW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectMT(makeRay(i),
														   P0, P1, P2,
														   uv, t);
		if (Res[i]) {
			PR_CHECK_TRUE(res);
		} else {
			PR_CHECK_FALSE(res);
		}
	}
}

PR_TEST("MT Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectMT(makeRay(i),
														   P0, P2, P1,
														   uv, t);
		if (Res[i]) {
			PR_CHECK_TRUE(res);
		} else {
			PR_CHECK_FALSE(res);
		}
	}
}
PR_TEST("PI Intersects CCW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_NonOpt(makeRayPackage(),
																promote(P0), promote(P1), promote(P2),
																uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI Intersects CW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_NonOpt(makeRayPackage(),
																promote(P0), promote(P2), promote(P1),
																uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI Intersects CCW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_NonOpt(makeRay(i),
																  P0, P1, P2,
																  uv, t);
		if (Res[i]) {
			PR_CHECK_TRUE(res);
		} else {
			PR_CHECK_FALSE(res);
		}
	}
}

PR_TEST("PI Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_NonOpt(makeRay(i),
																  P0, P2, P1,
																  uv, t);
		if (Res[i]) {
			PR_CHECK_TRUE(res);
		} else {
			PR_CHECK_FALSE(res);
		}
	}
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Triangle);
PRT_END_MAIN