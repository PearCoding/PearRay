#include "math/AutoDiff.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Auto_Diff)
PR_TEST("Add Scalar")
{
	DxF f1(1.0, 0.0f);
	DxF f2(0.0, 1.0f);

	DxF n = f1 + f2;

	PR_CHECK_EQ(n.v(), 1);
	PR_CHECK_EQ(n.d(0), 1);
}
PR_TEST("Sub Scalar")
{
	DxF f1(1.0, { 0.0f });
	DxF f2(0.0, { 1.0f });

	auto n = f1 - f2;

	PR_CHECK_EQ(n.v(), 1);
	PR_CHECK_EQ(n.d(0), -1);
}
PR_TEST("Mul Scalar")
{
	DxF f1(1.0, { 0.0f });
	DxF f2(0.0, { 1.0f });

	auto n = f1 * f2;

	PR_CHECK_EQ(n.v(), 0);
	PR_CHECK_EQ(n.d(0), 1);

	auto n2 = f1 * 22;
	PR_CHECK_EQ(n2.v(), 22);
	PR_CHECK_EQ(n2.d(0), 0);

	auto n3 = 22 * f1;
	PR_CHECK_EQ(n3.v(), 22);
	PR_CHECK_EQ(n3.d(0), 0);
}
PR_TEST("Div Scalar")
{
	DxF f1(1.0, { 0.0f });
	DxF f2(2.0, { 1.0f });

	auto n = f1 / f2;

	PR_CHECK_EQ(n.v(), 0.5);
	PR_CHECK_EQ(n.d(0), -0.25);

	auto n2 = f1 / 2;
	PR_CHECK_EQ(n2.v(), 0.5);
	PR_CHECK_EQ(n2.d(0), 0);

	auto n3 = 1 / f2;
	PR_CHECK_EQ(n3.v(), 0.5);
	PR_CHECK_EQ(n3.d(0), -0.25);
}
PR_TEST("Abs Scalar")
{
	DxF f1(1.0, { -2.0f });
	DxF f2(-4.0, { -1.0f });

	auto n1 = f1.abs();
	auto n2 = f2.abs();

	PR_CHECK_EQ(n1.v(), 1);
	PR_CHECK_EQ(n1.d(0), -2);

	PR_CHECK_EQ(n2.v(), 4);
	PR_CHECK_EQ(n2.d(0), 1);
}
PR_TEST("Sqrt Scalar")
{
	DxF f(4, 1);
	auto n = sqrt(f);
	PR_CHECK_EQ(n.v(), 2);
	PR_CHECK_EQ(n.d(0), 0.25);
}
PR_TEST("Cbrt Scalar")
{
	DxF f(8, 1);
	auto n = cbrt(f);
	PR_CHECK_EQ(n.v(), 2);
	PR_CHECK_NEARLY_EQ(n.d(0), 1/12.0);
}
PR_TEST("Sin Scalar")
{
	DxF f(PR_PI, 1);
	DxF n = sin(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), -1);
}
PR_TEST("Arcus Sin Scalar")
{
	DxF f(0, 1);
	DxF n = asin(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), 1);
}
PR_TEST("Cos Scalar")
{
	DxF f(PR_PI/2, 1);
	DxF n = cos(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), -1);
}
PR_TEST("Arcus Cos Scalar")
{
	DxF f(0, 1);
	DxF n = acos(f);
	PR_CHECK_NEARLY_EQ(n.v(), PR_PI/2);
	PR_CHECK_NEARLY_EQ(n.d(0), -1);
}
PR_TEST("Tan Scalar")
{
	DxF f(0, 1);
	DxF n = tan(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), 1);
}
PR_TEST("Arcus Tan Scalar")
{
	DxF f(0, 1);
	DxF n = atan(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), 1);
}
PR_TEST("Exp Scalar")
{
	DxF f(0, 1);
	DxF n = exp(f);
	PR_CHECK_NEARLY_EQ(n.v(), 1);
	PR_CHECK_NEARLY_EQ(n.d(0), 1);
}
PR_TEST("Log Scalar")
{
	DxF f(1, 1);
	DxF n = log(f);
	PR_CHECK_NEARLY_EQ(n.v(), 0);
	PR_CHECK_NEARLY_EQ(n.d(0), 1);
}
PR_TEST("Add Vector")
{
	DxV3F f1(Eigen::Vector3f(1, 2, 3), Eigen::Vector3f(0,1,2));
	DxV3F f2(Eigen::Vector3f(4, 5, 6), Eigen::Vector3f(1,2,3));

	DxV3F n = f1 + f2;

	PR_CHECK_EQ(n.v(), Eigen::Vector3f(5,7,9));
	PR_CHECK_EQ(n.d(0), Eigen::Vector3f(1,3,5));
}
PR_TEST("Sub Vector")
{
	DxV3F f1(Eigen::Vector3f(1, 2, 3), Eigen::Vector3f(0,1,2));
	DxV3F f2(Eigen::Vector3f(4, 5, 6), Eigen::Vector3f(1,2,3));

	DxV3F n = f1 - f2;

	PR_CHECK_EQ(n.v(), Eigen::Vector3f(-3,-3,-3));
	PR_CHECK_EQ(n.d(0), Eigen::Vector3f(-1,-1,-1));
}
PR_END_TESTCASE()

PRT_BEGIN_MAIN
PRT_TESTCASE(Auto_Diff);
PRT_END_MAIN