#include "math/Curve.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Curve)

PR_TEST("Simple")
{
	Curve2<2> curve({ Vector2f(0, 0), Vector2f(0.5, 1), Vector2f(1, 0) });

	PR_CHECK_NEARLY_EQ(curve.eval(0.5), Vector2f(0.5, 0.5));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Curve);
PRT_END_MAIN