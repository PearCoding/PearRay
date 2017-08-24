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

PR_TEST("Cos Hemi Normalized")
{
	const float u1 = 0.5f;
	const float u2 = 0.5f;

	float pdf1;
	auto V1 = Projection::cos_hemi(u1, u2, pdf1);
	PR_CHECK_NEARLY_EQ(V1.squaredNorm(), 1)

	for(int i = 0; i < 10; ++i)
	{
		float pdf2;
		auto V2 = Projection::cos_hemi(u1, u2, 1, pdf2);
		PR_CHECK_NEARLY_EQ(V2.squaredNorm(), 1)
	}
}

PR_TEST("Cos Hemi Equal")
{
	const float u1 = 0.5f;
	const float u2 = 0.5f;

	float pdf1;
	auto V1 = Projection::cos_hemi(u1, u2, pdf1);
	
	float pdf2;
	auto V2 = Projection::cos_hemi(u1, u2, 1, pdf2);

	auto N = Eigen::Vector3f(0, 0, 1);
	float pdf3 = Projection::cos_hemi_pdf(V1.dot(N));
	float pdf4 = Projection::cos_hemi_pdf(V2.dot(N));

	PR_CHECK_NEARLY_EQ(V1, V2);
	PR_CHECK_NEARLY_EQ(pdf1, pdf2);
	PR_CHECK_NEARLY_EQ(pdf2, pdf3);
	PR_CHECK_NEARLY_EQ(pdf3, pdf4);
	PR_CHECK_NEARLY_EQ(pdf4, pdf1);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Projection);
PRT_END_MAIN