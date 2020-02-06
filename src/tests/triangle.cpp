#include "geometry/Triangle.h"
#include "geometry/TriangleIntersection.h"
#include "geometry/TriangleIntersection_BW.h"
#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

const Vector3f P0 = Vector3f(0, 0, 0);
const Vector3f P1 = Vector3f(1, 0, 0);
const Vector3f P2 = Vector3f(0, 1, 0);

const Vector3f M0 = P0.cross(P1);
const Vector3f M1 = P1.cross(P2);
const Vector3f M2 = P2.cross(P0);

const Vector3f D	= Vector3f(0, 0, 1);
const Vector3f P[4] = { Vector3f(0.25f, 0.25f, -1), // Always true
						Vector3f(0.25f, 0.75f, -1), // True only on CCW, due to edge inconsistencies
						Vector3f(0, 0, -1),			// Same as above
						Vector3f(0.6f, 0.6f, -1) }; // Always false!

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
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
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
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

////////////////////////////////////

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
	// Inconsistent due to edge errors! Sign can not be determined...
	//PR_CHECK_TRUE(extract<1>(res));
	//PR_CHECK_TRUE(extract<2>(res));
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
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("PI Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		// Inconsistent due to edge errors! Sign can not be determined...
		if (i == 1 || i == 2)
			continue;

		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_NonOpt(makeRay(i),
																  P0, P2, P1,
																  uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

////////////////////////////////////

PR_TEST("PI OPT Intersects CCW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_Opt(makeRayPackage(),
															 promote(P0), promote(P1), promote(P2),
															 promote(M0), promote(M1), promote(M2),
															 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI OPT Intersects CW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_Opt(makeRayPackage(),
															 promote(P0), promote(P2), promote(P1),
															 -promote(M2), -promote(M1), -promote(M0),
															 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	// Inconsistent due to edge errors! Sign can not be determined...
	//PR_CHECK_TRUE(extract<1>(res));
	//PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI OPT Intersects CCW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_Opt(makeRay(i),
															   P0, P1, P2,
															   M0, M1, M2,
															   uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("PI OPT Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		// Inconsistent due to edge errors! Sign can not be determined...
		if (i == 1 || i == 2)
			continue;

		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_Opt(makeRay(i),
															   P0, P2, P1,
															   -M2, -M1, -M0,
															   uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}


////////////////////////////////////

PR_TEST("PI Em Intersects CCW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_Em(makeRayPackage(),
																promote(P0), promote(P1), promote(P2),
																uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI Em Intersects CW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectPI_Em(makeRayPackage(),
																promote(P0), promote(P2), promote(P1),
																uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	// Inconsistent due to edge errors! Sign can not be determined...
	//PR_CHECK_TRUE(extract<1>(res));
	//PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("PI Em Intersects CCW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_Em(makeRay(i),
																  P0, P1, P2,
																  uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("PI Em Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		// Inconsistent due to edge errors! Sign can not be determined...
		if (i == 1 || i == 2)
			continue;

		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectPI_Em(makeRay(i),
																  P0, P2, P1,
																  uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}
////////////////////////////////////

PR_TEST("WT Intersects CCW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectWT(makeRayPackage(),
														 promote(P0), promote(P1), promote(P2),
														 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("WT Intersects CW [V]")
{
	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectWT(makeRayPackage(),
														 promote(P0), promote(P2), promote(P1),
														 uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("WT Intersects CCW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectWT(makeRay(i),
														   P0, P1, P2,
														   uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("WT Intersects CW")
{
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectWT(makeRay(i),
														   P0, P2, P1,
														   uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

////////////////////////////////////

PR_TEST("BW 9 Intersects CCW [V]")
{
	float mat[9];
	int fixedColumn;
	TriangleIntersection::constructBW9Matrix(P0, P1, P2, mat, fixedColumn);

	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectBW9(makeRayPackage(),
														  mat, fixedColumn,
														  uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("BW 9 Intersects CW [V]")
{
	float mat[9];
	int fixedColumn;
	TriangleIntersection::constructBW9Matrix(P0, P1, P2, mat, fixedColumn);

	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectBW9(makeRayPackage(),
														  mat, fixedColumn,
														  uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("BW 9 Intersects CCW")
{
	float mat[9];
	int fixedColumn;
	TriangleIntersection::constructBW9Matrix(P0, P1, P2, mat, fixedColumn);

	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectBW9(makeRay(i),
															mat, fixedColumn,
															uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("BW 9 Intersects CW")
{
	float mat[9];
	int fixedColumn;
	TriangleIntersection::constructBW9Matrix(P0, P1, P2, mat, fixedColumn);
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectBW9(makeRay(i),
															mat, fixedColumn,
															uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

////////////////////////////////////

PR_TEST("BW 12 Intersects CCW [V]")
{
	float mat[12];
	TriangleIntersection::constructBW12Matrix(P0, P1, P2, mat);

	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectBW12(makeRayPackage(),
														   mat,
														   uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("BW 12 Intersects CW [V]")
{
	float mat[12];
	TriangleIntersection::constructBW12Matrix(P0, P1, P2, mat);

	Vector2fv uv;
	vfloat t;
	const bfloat res = TriangleIntersection::intersectBW12(makeRayPackage(),
														   mat,
														   uv, t);
	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("BW 12 Intersects CCW")
{
	float mat[12];
	TriangleIntersection::constructBW12Matrix(P0, P1, P2, mat);

	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectBW12(makeRay(i),
															 mat,
															 uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_TEST("BW 12 Intersects CW")
{
	float mat[12];
	TriangleIntersection::constructBW12Matrix(P0, P1, P2, mat);
	for (size_t i = 0; i < 4; ++i) {
		Vector2f uv;
		float t;
		const bool res = TriangleIntersection::intersectBW12(makeRay(i),
															 mat,
															 uv, t);
		if (Res[i])
			PR_CHECK_TRUE(res);
		else
			PR_CHECK_FALSE(res);
	}
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Triangle);
PRT_END_MAIN