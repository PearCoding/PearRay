#include "sampler/Projection.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sampler)
PR_TEST("Align 1");
{
	auto N = PM::pm_Set(0, 0, 1);
	auto V = PM::pm_Set(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ_3(R, V);
}

PR_TEST("Align 2");
{
	auto N = PM::pm_Set(0, 1, 0);
	auto V = PM::pm_Set(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ_3(R, PM::pm_Set(0, 0, 1));
}

PR_TEST("Align 3");
{
	auto N = PM::pm_Normalize3D(PM::pm_Set(0, 0.5f, 0.5f));
	auto V = PM::pm_Set(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ_3(R, PM::pm_Normalize3D(PM::pm_Set(0, 0.5f, 0.5f)));
}

PR_TEST("Align 4");
{
	auto N = PM::pm_Set(1, 0, 0);
	auto V = PM::pm_Set(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ_3(R, PM::pm_Set(0, 1, 0));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sampler);
PRT_END_MAIN