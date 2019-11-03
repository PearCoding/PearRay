#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Vector)
PR_TEST("sin")
{
	const float V = PR_PI;
	const float R = sin(V);

	vfloat VV = vfloat(V);
	vfloat VR = sin(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("cos")
{
	const float V = 0;
	const float R = cos(V);

	vfloat VV = vfloat(V);
	vfloat VR = cos(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("tan")
{
	const float V = 0;
	const float R = tan(V);

	vfloat VV = vfloat(V);
	vfloat VR = tan(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("asin")
{
	const float V = 1;
	const float R = asin(V);

	vfloat VV = vfloat(V);
	vfloat VR = asin(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("acos")
{
	const float V = 0;
	const float R = acos(V);

	vfloat VV = vfloat(V);
	vfloat VR = acos(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("atan")
{
	const float V = 1;
	const float R = atan(V);

	vfloat VV = vfloat(V);
	vfloat VR = atan(VV);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_TEST("atan2")
{
	const float V1 = 1;
	const float V2 = 0;
	const float R  = atan2(V1, V2);

	vfloat VV1 = vfloat(V1);
	vfloat VV2 = vfloat(V2);
	vfloat VR  = atan2(VV1, VV2);

	PR_CHECK_NEARLY_EQ(extract<0>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<1>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<2>(VR), R);
	PR_CHECK_NEARLY_EQ(extract<3>(VR), R);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Vector);
PRT_END_MAIN