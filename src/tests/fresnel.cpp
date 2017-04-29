#include "math/Fresnel.h"
#include "Test.h"

using namespace PR;

#ifdef PR_DEBUG
# ifndef PR_TEST_VERBOSE
#  define PR_TEST_VERBOSE
# endif
#endif

PR_BEGIN_TESTCASE(Dielectric)
PR_TEST("Zero Dot")
{
	float R = Fresnel::dielectric(0,1,1);
	PR_CHECK_NEARLY_EQ(R, 1);
}
PR_TEST("One Dot")
{
	float R = Fresnel::dielectric(1,1,1);
	PR_CHECK_NEARLY_EQ(R, 0);
}
PR_END_TESTCASE()

PR_BEGIN_TESTCASE(Conductor)
PR_TEST("Zero Dot")
{
	float R = Fresnel::conductor(0,1,1);
	PR_CHECK_NEARLY_EQ(R, 1);
}
PR_TEST("One Dot")
{
	float R = Fresnel::conductor(1,1,1);
	PR_CHECK_NEARLY_EQ(R, 0.02);
}
PR_END_TESTCASE()

PR_BEGIN_TESTCASE(Schlick)
PR_TEST("Zero Dot")
{
	float R = Fresnel::schlick(0,1,1);
	PR_CHECK_NEARLY_EQ(R, 1);
}
PR_TEST("One Dot")
{
	float R = Fresnel::schlick(1,1,1);
	PR_CHECK_NEARLY_EQ(R, 0);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Dielectric);
PRT_TESTCASE(Conductor);
PRT_TESTCASE(Schlick);
PRT_END_MAIN