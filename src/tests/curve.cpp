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

PR_TEST("Derivative")
{
	Curve2 curve({ Vector2f(1, 1), Vector2f(3, 1), Vector2f(4, 2), Vector2f(6, 3) });

	Vector2f ds = 3 * (curve.point(1) - curve.point(0));
	Vector2f de = 3 * (curve.point(3) - curve.point(2));
	PR_CHECK_NEARLY_EQ(curve.evalDerivative(0), ds);
	PR_CHECK_NEARLY_EQ(curve.evalDerivative(0.5f), Vector2f(4.5f, 2.25f));
	PR_CHECK_NEARLY_EQ(curve.evalDerivative(1), de);
}

PR_TEST("Blossom")
{
	Curve2 curve({ Vector2f(1, 1), Vector2f(3, 1), Vector2f(4, 2), Vector2f(6, 3) });

	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 0, 0 }), curve.point(0));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 0, 1 }), curve.point(1));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 1, 1 }), curve.point(2));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 1, 1, 1 }), curve.point(3));
}

PR_TEST("Blossom Fast")
{
	Curve2 curve({ Vector2f(1, 1), Vector2f(3, 1), Vector2f(4, 2), Vector2f(6, 3) });
	Curve2 res = curve.blossom(0, 1);

	PR_CHECK_NEARLY_EQ(res.point(0), curve.point(0));
	PR_CHECK_NEARLY_EQ(res.point(1), curve.point(1));
	PR_CHECK_NEARLY_EQ(res.point(2), curve.point(2));
	PR_CHECK_NEARLY_EQ(res.point(3), curve.point(3));
}

PR_TEST("Blossom Eq")
{
	Curve2 curve({ Vector2f(1, 1), Vector2f(3, 1), Vector2f(4, 2), Vector2f(6, 3) });
	Curve2 res = curve.blossom(0, 0.5f);

	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 0, 0 }), res.point(0));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 0, 0.5f }), res.point(1));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0, 0.5f, 0.5f }), res.point(2));
	PR_CHECK_NEARLY_EQ(curve.evalBlossom({ 0.5f, 0.5f, 0.5f }), res.point(3));
}

PR_TEST("Subdivision")
{
	Curve2 curve({ Vector2f(1, 1), Vector2f(3, 1), Vector2f(4, 2), Vector2f(6, 3) });
	Curve2 res0 = curve.blossom(0, 0.5f);
	Curve2 res1 = curve.blossom(0.5f, 1);

	Curve2 firstHalf, secondHalf;
	curve.subdivide(firstHalf, secondHalf);

	PR_CHECK_NEARLY_EQ(firstHalf.point(0), res0.point(0));
	PR_CHECK_NEARLY_EQ(firstHalf.point(1), res0.point(1));
	PR_CHECK_NEARLY_EQ(firstHalf.point(2), res0.point(2));
	PR_CHECK_NEARLY_EQ(firstHalf.point(3), res0.point(3));

	PR_CHECK_NEARLY_EQ(secondHalf.point(0), res1.point(0));
	PR_CHECK_NEARLY_EQ(secondHalf.point(1), res1.point(1));
	PR_CHECK_NEARLY_EQ(secondHalf.point(2), res1.point(2));
	PR_CHECK_NEARLY_EQ(secondHalf.point(3), res1.point(3));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Curve);
PRT_END_MAIN