#include "geometry/Sphere.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sphere)
PR_TEST("Size");
{
	Sphere sphere(PM::pm_Set(0,0,0), 1);
	PR_CHECK_NEARLY_EQ(sphere.volume(), 4*PM_PI_F/3);
}

PR_TEST("Intersects");
{
	Ray ray(PM::pm_Zero(), PM::pm_Set(-2, 0, 0), PM::pm_Set(1, 0, 0));
	Sphere sphere(PM::pm_Set(0, 0, 0), 1);

	PM::vec3 collisionPoint;
	float t;
	sphere.intersects(ray, collisionPoint, t);
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(-1, 0, 0));
}

PR_TEST("Intersects Inside");
{
	Ray ray(PM::pm_Zero(), PM::pm_Set(0, 0, 0), PM::pm_Set(1, 0, 0));
	Sphere sphere(PM::pm_Set(0, 0, 0), 1);

	PM::vec3 collisionPoint;
	float t;
	sphere.intersects(ray, collisionPoint, t);
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(1, 0, 0));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sphere);
PRT_END_MAIN