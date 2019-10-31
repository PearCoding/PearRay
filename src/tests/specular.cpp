#include "math/Specular.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Specular)
PR_TEST("Backmann")
{
	PR_CHECK_NEARLY_EQ(Specular::beckmann(0, 1), 0);
	PR_CHECK_NEARLY_EQ(Specular::beckmann(1, 0), 0);
	PR_CHECK_NEARLY_EQ(Specular::beckmann(1, 1), PR_1_PI);
}
PR_END_TESTCASE()

PRT_BEGIN_MAIN
PRT_TESTCASE(Specular);
PRT_END_MAIN