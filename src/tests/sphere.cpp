#include "geometry/Sphere.h"
#include "geometry/Plane.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sphere)
PR_TEST("Size")
{
	Sphere sphere(Eigen::Vector3f(0, 0, 0), 1);
	PR_CHECK_NEARLY_EQ(sphere.volume(), 4 * PR_PI / 3);
}

PR_TEST("Intersects")
{
	Sphere sphere(Eigen::Vector3f(0, 0, 0), 1);
	Ray ray(-2, 0, 0,
			1, 0, 0);

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_TEST("Intersects Inside")
{
	Sphere sphere(Eigen::Vector3f(0, 0, 0), 1);
	Ray ray(0, 0, 0,
			1, 0, 0);

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sphere);
PRT_END_MAIN