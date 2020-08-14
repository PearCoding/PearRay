#include "math/Concentric.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Concentric)

PR_TEST("RPhi")
{
	Vector2f p = Concentric::square2rphi(Vector2f(0, 0));
	Vector2f r = Concentric::rphi2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0, 0));

	p = Concentric::square2rphi(Vector2f(0.5f, 0.5f));
	r = Concentric::rphi2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.5f, 0.5f));

	p = Concentric::square2rphi(Vector2f(0.25f, 0.75f));
	r = Concentric::rphi2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.25f, 0.75f));

	p = Concentric::square2rphi(Vector2f(0.75f, 0.25f));
	r = Concentric::rphi2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.75f, 0.25f));

	p = Concentric::square2rphi(Vector2f(0.184375f, 0.916725f));
	r = Concentric::rphi2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.184375f, 0.916725f));
}

PR_TEST("Disc")
{
	Vector2f p = Concentric::square2disc(Vector2f(0, 0));
	Vector2f r = Concentric::disc2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0, 0));

	p = Concentric::square2disc(Vector2f(0.5f, 0.5f));
	r = Concentric::disc2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.5f, 0.5f));

	p = Concentric::square2disc(Vector2f(0.25f, 0.75f));
	r = Concentric::disc2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.25f, 0.75f));

	p = Concentric::square2disc(Vector2f(0.75f, 0.25f));
	r = Concentric::disc2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.75f, 0.25f));

	p = Concentric::square2disc(Vector2f(0.184375f, 0.916725f));
	r = Concentric::disc2square(p);
	PR_CHECK_NEARLY_EQ(r, Vector2f(0.184375f, 0.916725f));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Concentric);
PRT_END_MAIN