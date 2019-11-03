#include "Random.h"

#include "Test.h"

#include <ctime>

using namespace PR;

constexpr uint64 BOUND_CHECK_N = 10000000;

PR_BEGIN_TESTCASE(Random)
PR_TEST("seed")
{
	uint32 seed = std::time(nullptr);
	Random r1(seed);
	Random r2(seed);

	for(uint32 i = 0; i < 20; ++i)
		PR_CHECK_EQ(r1.get64(), r2.get64());
}

PR_TEST("float")
{
	Random r(std::time(nullptr));

	bool outofbound = false;
	for(uint64 i = 0; i < BOUND_CHECK_N; ++i)
	{
		float f = r.getFloat();
		if(f < 0 || f > 1)
		{
			outofbound = true;
			break;
		}
	}

	PR_CHECK_FALSE(outofbound);
}

PR_TEST("double")
{
	Random r(std::time(nullptr));

	bool outofbound = false;
	for(uint64 i = 0; i < BOUND_CHECK_N; ++i)
	{
		double f = r.getDouble();
		if(f < 0 || f > 1)
		{
			outofbound = true;
			break;
		}
	}

	PR_CHECK_FALSE(outofbound);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Random);
PRT_END_MAIN