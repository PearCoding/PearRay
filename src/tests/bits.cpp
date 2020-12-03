#include "math/Bits.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Bits)
PR_TEST("unpack/pack even")
{
	const uint32 rx = 1548;
	PR_CHECK_EQ(unpack_even(pack_even(rx)), rx);
}
PR_TEST("unpack/pack k3")
{
	const uint32 rx = 1548;
	PR_CHECK_EQ(unpack_k3(pack_k3(rx)), rx);
}
PR_TEST("Morton 2D")
{
	uint32 rx = 42;
	uint32 ry = 56;

	uint64 morton = xy_2_morton(rx, ry);

	uint32 x, y;
	morton_2_xy(morton, x, y);
	PR_CHECK_EQ(x, rx);
	PR_CHECK_EQ(y, ry);
}
PR_TEST("Morton 3D")
{
	uint32 rx = 406789;
	uint32 ry = 112488;
	uint32 rz = 787233;

	uint64 morton = xyz_2_morton(rx, ry, rz);

	uint32 x, y, z;
	morton_2_xyz(morton, x, y, z);
	PR_CHECK_EQ(x, rx);
	PR_CHECK_EQ(y, ry);
	PR_CHECK_EQ(z, rz);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Bits);
PRT_END_MAIN