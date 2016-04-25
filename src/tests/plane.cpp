#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Plane)
PR_TEST("Size");
{
	Plane plane(10, 10);
	PR_CHECK_EQ(plane.width(), 10);
	PR_CHECK_EQ(plane.height(), 10);
}

PR_TEST("Axis");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(10, 0, 0, 1), PM::pm_Set(0, 10, 0, 1));
	PR_CHECK_EQ(plane.width(), 10);
	PR_CHECK_EQ(plane.height(), 10);
}

PR_TEST("Normal");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
	PM::vec3 norm = plane.normal();
	PR_CHECK_NEARLY_EQ_3(norm, PM::pm_Set(0, 0, -1));
}

PR_TEST("Normal 2");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 0, 1, 1));
	PM::vec3 norm = plane.normal();
	PR_CHECK_NEARLY_EQ_3(norm, PM::pm_Set(0, 1, 0));
}

PR_TEST("Center");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
	PM::vec3 center = plane.center();
	PR_CHECK_NEARLY_EQ_3(center, PM::pm_Set(0.5, 0.5, 0));
}

PR_TEST("Contains");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
	PR_CHECK_TRUE(plane.contains(PM::pm_Set(0.5, 0.5, 0, 1)));
	PR_CHECK_FALSE(plane.contains(PM::pm_Set(-0.5, 0.5, 0, 1)));
	PR_CHECK_TRUE(plane.contains(PM::pm_Set(0.5, 0.5, 0, 0)));
	PR_CHECK_FALSE(plane.contains(PM::pm_Set(-0.5, 0.5, 0, 0)));
}

PR_TEST("Contains 2");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(10, 0, 0, 1), PM::pm_Set(0, 10, 0, 1));
	PR_CHECK_TRUE(plane.contains(PM::pm_Set(5, 5, 0, 1)));
	PR_CHECK_FALSE(plane.contains(PM::pm_Set(-5, 5, 0, 1)));
	PR_CHECK_TRUE(plane.contains(PM::pm_Set(5, 5, 0, 0)));
	PR_CHECK_FALSE(plane.contains(PM::pm_Set(-5, 5, 0, 0)));
}

PR_TEST("Intersects 1");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
	Ray ray(PM::pm_Set(0.5, 0.5, -1, 1), PM::pm_Set(0, 0, 1, 0));

	PM::vec3 point;
	float u, v;
	PR_CHECK_TRUE(plane.intersects(ray, point, u, v));
	PR_CHECK_NEARLY_EQ_3(point, PM::pm_Set(0.5, 0.5, 0));
	PR_CHECK_NEARLY_EQ(u, 0.5);
	PR_CHECK_NEARLY_EQ(v, 0.5);
}

PR_TEST("Intersects 2");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(1, 0, 0, 1), PM::pm_Set(0, 1, 0, 1));
	Ray ray(PM::pm_Set(0.5, 0.5, -1, 1), PM::pm_Set(0, 1, 0, 0));

	PR_CHECK_FALSE(plane.intersects(ray));
}

PR_TEST("Intersects 3");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(10, 0, 0, 1), PM::pm_Set(0, 10, 0, 1));
	Ray ray(PM::pm_Set(5, 5, -1, 1), PM::pm_Set(0, 0, 1, 0));

	PM::vec3 point;
	float u, v;
	PR_CHECK_TRUE(plane.intersects(ray, point, u, v));
	PR_CHECK_NEARLY_EQ_3(point, PM::pm_Set(5, 5, 0));
	PR_CHECK_NEARLY_EQ(u, 0.5);
	PR_CHECK_NEARLY_EQ(v, 0.5);
}

PR_TEST("Intersects 4");
{
	Plane plane(PM::pm_Set(0, 0, 0, 1), PM::pm_Set(10, 0, 0, 1), PM::pm_Set(0, 20, 0, 1));
	Ray ray(PM::pm_Set(5, 10, -1, 1), PM::pm_Set(0, 0, 1, 0));

	PM::vec3 point;
	float u, v;
	PR_CHECK_TRUE(plane.intersects(ray, point, u, v));
	PR_CHECK_NEARLY_EQ_3(point, PM::pm_Set(5, 10, 0));
	PR_CHECK_NEARLY_EQ(u, 0.5);
	PR_CHECK_NEARLY_EQ(v, 0.5);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Plane);
PRT_END_MAIN