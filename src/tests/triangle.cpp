#include "geometry/Triangle.h"
#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Triangle)
PR_TEST("Intersects CCW")
{
	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.5, 0.25, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.75, 0, 0.6);
	in.Origin[2] = simdpp::make_float(-1, -1, -1, -1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(1);

	in.setupInverse();
	
	vfloat u, v, t;
	const bfloat res = Triangle::intersect(in,
										   0, 0, 0,
										   1, 0, 0,
										   0, 1, 0,
										   u, v, t);

	PR_CHECK_TRUE(any(res));
	PR_CHECK_FALSE(all(res));

	PR_CHECK_TRUE(extract<0>(res));
	PR_CHECK_TRUE(extract<1>(res));
	PR_CHECK_TRUE(extract<2>(res));
	PR_CHECK_FALSE(extract<3>(res));
}

PR_TEST("Intersects CW")
{
	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.5, 0.25, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.75, 0, 0.6);
	in.Origin[2] = simdpp::make_float(-1, -1, -1, -1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(1);

	in.setupInverse();

	vfloat u, v, t;
	const bfloat res = Triangle::intersect(in,
										   0, 0, 0,
										   0, 1, 0,
										   1, 0, 0,
										   u, v, t);

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