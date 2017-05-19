#include "math/Projection.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Projection)
PR_TEST("Align 1")
{
	auto N = Eigen::Vector3f(0, 0, 1);
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, V);
}

PR_TEST("Align 2")
{
	auto N = Eigen::Vector3f(0, 1, 0);
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Eigen::Vector3f(0, 0, -1));
}

PR_TEST("Align 3")
{
	auto N = Eigen::Vector3f(0, 0.5f, 0.5f).normalized();
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Eigen::Vector3f(0, 0.5f, -0.5f).normalized());
}

PR_TEST("Align 4")
{
	auto N = Eigen::Vector3f(1, 0, 0);
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Eigen::Vector3f(0, 1, 0));
}

PR_TEST("Align 5")
{
	auto N = Eigen::Vector3f(0, 1, 0);
	auto V = Eigen::Vector3f(0, 0, 1);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, N);
}

PR_TEST("Align 6")
{
	auto N = Eigen::Vector3f(0, 0, -1);
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Eigen::Vector3f(0, -1, 0));
}

PR_TEST("Align 6")
{
	auto N = Eigen::Vector3f(0, -1, 0);
	auto V = Eigen::Vector3f(0, 1, 0);

	auto R = Projection::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Eigen::Vector3f(0, 0, 1));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Projection);
PRT_END_MAIN