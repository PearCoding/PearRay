#include "math/Compression.h"

#define PRT_EPSILON (0.00001f)
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Compression)
PR_TEST("unorm16")
{
	float f   = 0.5f;
	unorm16 s = to_unorm16(f);
	float r   = from_unorm16(s);

	PR_CHECK_NEARLY_EQ(r, f);
}

PR_TEST("snorm16")
{
	float f   = -0.25f;
	snorm16 s = to_snorm16(f);
	float r   = from_snorm16(s);

	PR_CHECK_NEARLY_EQ(r, f);
}

PR_TEST("oct 1")
{
	Vector3f d(1, 0, 0);
	Vector2f o = to_oct(d);
	Vector3f r = from_oct(o);

	PR_CHECK_NEARLY_EQ(r, d);
}

PR_TEST("oct 2")
{
	Vector3f d(0, -1, 0);
	Vector2f o = to_oct(d);
	Vector3f r = from_oct(o);

	PR_CHECK_NEARLY_EQ(r, d);
}

PR_TEST("oct 3")
{
	Vector3f d(0.707107f, 0, 0.707107f);
	Vector2f o = to_oct(d);
	Vector3f r = from_oct(o);

	PR_CHECK_NEARLY_EQ(r, d);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Compression);
PRT_END_MAIN