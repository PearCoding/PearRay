#include "math/Normal.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Normal)

PR_TEST("ErfInv")
{
	const float e  = std::erf(0.5f);
	const float ie = Normal::erfinv(e);
	PR_CHECK_NEARLY_EQ(ie, 0.5f);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Normal);
PRT_END_MAIN