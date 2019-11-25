#include "curve/Curve.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Curve)

PR_TEST("Simple")
{
	Curve2 curve({ Vector2f(0, 0), Vector2f(0.5f, 1), Vector2f(1, 0) });

	PR_CHECK_NEARLY_EQ(curve.eval(0), Vector2f(0, 0));
	PR_CHECK_NEARLY_EQ(curve.eval(0.5f), Vector2f(0.5f, 0.5f));
	PR_CHECK_NEARLY_EQ(curve.eval(1), Vector2f(1, 0));
}

PR_TEST("Degree 2")
{
	Curve2 curve({ Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1) });

	PR_CHECK_NEARLY_EQ(curve.eval(0), Vector2f(0, 0));
	PR_CHECK_NEARLY_EQ(curve.eval(0.5f), Vector2f(0.5f, 0.75f));
	PR_CHECK_NEARLY_EQ(curve.eval(1), Vector2f(0, 1));
}

PR_TEST("Degree 3")
{
	Curve2 curve({ Vector2f(0, 0), Vector2f(1, 1), Vector2f(2, 0), Vector2f(3, 1) });

	PR_CHECK_NEARLY_EQ(curve.eval(0), Vector2f(0, 0));
	PR_CHECK_NEARLY_EQ(curve.eval(0.5f), Vector2f(1.5f, 0.5f));
	PR_CHECK_NEARLY_EQ(curve.eval(1), Vector2f(3, 1));
}

PR_TEST("3D proj 2D")
{
	Curve3 curve({ Vector3f(0, 0, 0), Vector3f(1, 1, 0), Vector3f(2, 0, 0), Vector3f(3, 1, 0) });

	PR_CHECK_NEARLY_EQ(curve.eval(0), Vector3f(0, 0, 0));
	PR_CHECK_NEARLY_EQ(curve.eval(0.5f), Vector3f(1.5f, 0.5f, 0));
	PR_CHECK_NEARLY_EQ(curve.eval(1), Vector3f(3, 1, 0));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Curve);
PRT_END_MAIN