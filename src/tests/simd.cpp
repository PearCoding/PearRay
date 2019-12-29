#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(SIMD)
PR_TEST("extract")
{
	vfloat f1 = simdpp::make_float(0, 1, 2, 3);
	PR_CHECK_NEARLY_EQ(extract(0, f1), 0.0f);
	PR_CHECK_NEARLY_EQ(extract(1, f1), 1.0f);
	PR_CHECK_NEARLY_EQ(extract(2, f1), 2.0f);
	PR_CHECK_NEARLY_EQ(extract(3, f1), 3.0f);

	vuint32 i1 = simdpp::make_uint(0, 1, 2, 3);
	PR_CHECK_EQ(extract(0, i1), 0);
	PR_CHECK_EQ(extract(1, i1), 1);
	PR_CHECK_EQ(extract(2, i1), 2);
	PR_CHECK_EQ(extract(3, i1), 3);
}

PR_TEST("insert")
{
	vfloat f1 = simdpp::make_float(0, 1, 2, 3);
	f1		  = insert(0, f1, 42);
	PR_CHECK_NEARLY_EQ(extract(0, f1), 42.0f);

	vuint32 i1 = simdpp::make_uint(0, 1, 2, 3);
	i1		   = insert(0, i1, 42);
	PR_CHECK_EQ(extract(0, i1), 42);
}

PR_TEST("load_from_container_linear")
{
	float c[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	vfloat f1;
	load_from_container_linear(f1, c);

	PR_CHECK_NEARLY_EQ(extract(0, f1), 0.0f);
	PR_CHECK_NEARLY_EQ(extract(1, f1), 1.0f);
	PR_CHECK_NEARLY_EQ(extract(2, f1), 2.0f);
	PR_CHECK_NEARLY_EQ(extract(3, f1), 3.0f);
}

PR_TEST("load_from_container")
{
	std::vector<float> c = { 0, -1, 1, -2, 2, -3, 3, 4 };
	vuint32 ind			 = simdpp::make_uint(0, 2, 4, 6);

	vfloat f1 = load_from_container(ind, c);

	PR_CHECK_NEARLY_EQ(extract(0, f1), 0.0f);
	PR_CHECK_NEARLY_EQ(extract(1, f1), 1.0f);
	PR_CHECK_NEARLY_EQ(extract(2, f1), 2.0f);
	PR_CHECK_NEARLY_EQ(extract(3, f1), 3.0f);
}

PR_TEST("store_into_container")
{
	std::vector<float> c(8);
	vuint32 ind = simdpp::make_uint(0, 2, 4, 6);
	vfloat f1   = simdpp::make_float(0, 1, 2, 3);

	store_into_container(ind, c, f1);

	PR_CHECK_NEARLY_EQ(c[0], 0.0f);
	PR_CHECK_NEARLY_EQ(c[2], 1.0f);
	PR_CHECK_NEARLY_EQ(c[4], 2.0f);
	PR_CHECK_NEARLY_EQ(c[6], 3.0f);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(SIMD);
PRT_END_MAIN