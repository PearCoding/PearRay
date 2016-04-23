#include "geometry/Plane.h"
#include "Test.h"

using namespace PR;

int main(int argc, char** argv)
{
	PR_BEGIN_TESTCASE("Plane");
	PR_TEST("Size");
	{
		Plane plane(10, 10);
		PR_CHECK_EQ(plane.width(), 10);
		PR_CHECK_EQ(plane.height(), 10);
	}

	PR_TEST("Axis");
	{
		Plane plane(PM::pm_Set(0,0,0,1), PM::pm_Set(10, 0, 0, 1), PM::pm_Set(0, 10, 0, 1));
		PR_CHECK_EQ(plane.width(), 10);
		PR_CHECK_EQ(plane.height(), 10);
	}

	PR_TEST("Normal");
	{
		Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
		PM::vec3 norm = plane.normal();
		PR_CHECK_NEARLY_EQ(PM::pm_GetX(norm), 0);
		PR_CHECK_NEARLY_EQ(PM::pm_GetY(norm), 0);
		PR_CHECK_NEARLY_EQ(PM::pm_GetZ(norm), -1);
	}

	PR_TEST("Normal2");
	{
		Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 0, 1, 1));
		PM::vec3 norm = plane.normal();
		PR_CHECK_NEARLY_EQ(PM::pm_GetX(norm), 0);
		PR_CHECK_NEARLY_EQ(PM::pm_GetY(norm), 1);
		PR_CHECK_NEARLY_EQ(PM::pm_GetZ(norm), 0);
	}

	PR_TEST("Center");
	{
		Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
		PM::vec3 center = plane.center();
		PR_CHECK_NEARLY_EQ(PM::pm_GetX(center), 0.5f);
		PR_CHECK_NEARLY_EQ(PM::pm_GetY(center), 0.5f);
		PR_CHECK_NEARLY_EQ(PM::pm_GetZ(center), 0);
	}

	PR_END_TESTCASE();
	return 0;
}