#include "math/Sampling.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sampling)
PR_TEST("Cos Hemi Normalized")
{
	const float u1 = 0.5f;
	const float u2 = 0.5f;

	auto V1 = Sampling::cos_hemi(u1, u2);
	PR_CHECK_NEARLY_EQ(V1.squaredNorm(), 1);

	for (int i = 0; i < 10; ++i) {
		auto V2 = Sampling::cos_hemi(u1, u2, 1);
		PR_CHECK_NEARLY_EQ(V2.squaredNorm(), 1);
	}
}

PR_TEST("Cos Hemi Equal")
{
	const float u1 = 0.5f;
	const float u2 = 0.5f;

	auto V1 = Sampling::cos_hemi(u1, u2);

	auto V2 = Sampling::cos_hemi(u1, u2, 1);

	PR_CHECK_NEARLY_EQ(V1, V2);
	PR_CHECK_NEARLY_EQ(Sampling::cos_hemi_pdf(V1(2)), Sampling::cos_hemi_pdf(V2(2), 1));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sampling);
PRT_END_MAIN