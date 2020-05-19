#include "math/Bits.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Bits)
PR_TEST("Morton")
{
	uint32 rx = 42;
	uint32 ry = 56;

	uint64 morton = xy_2_morton(rx, ry);

	uint32 x, y;
	morton_2_xy(morton, x, y);
	PR_CHECK_EQ(x, rx);
	PR_CHECK_EQ(y, ry);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Bits);
PRT_END_MAIN