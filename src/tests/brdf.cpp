#include "material/BRDF.h"
#include "math/Reflection.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(BRDF)
PR_TEST("Reflection");
{
	auto N = PM::pm_Set(0, 1, 0);
	auto V = PM::pm_Normalize3D(PM::pm_Set(1, -1, 0));

	auto R = Reflection::reflect(PM::pm_Dot3D(V, N), N, V);

	PR_CHECK_NEARLY_EQ_3(R, PM::pm_Set(PM::pm_GetX(V), -PM::pm_GetY(V), PM::pm_GetZ(V)));
}

PR_TEST("Refraction");
{
	auto N = PM::pm_Set(0, 1, 0);
	auto V = PM::pm_Normalize3D(PM::pm_Set(1, -1, 0));

	auto R = Reflection::refract(1, PM::pm_Dot3D(V, N), N, V);

	PR_CHECK_NEARLY_EQ_3(R, PM::pm_Normalize3D(PM::pm_Set(1, -1, 0)));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BRDF);
PRT_END_MAIN