#include "geometry/Sphere.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sphere)
PR_TEST("Size")
{
	Sphere sphere(Eigen::Vector3f(0,0,0), 1);
	PR_CHECK_NEARLY_EQ(sphere.volume(), 4*PR_PI/3);
}

PR_TEST("Intersects")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(-2, 0, 0), Eigen::Vector3f(1, 0, 0));
	Sphere sphere(Eigen::Vector3f(0, 0, 0), 1);

	Sphere::Intersection s = sphere.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(-1, 0, 0));
}

PR_TEST("Intersects Inside")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0));
	Sphere sphere(Eigen::Vector3f(0, 0, 0), 1);

	Sphere::Intersection s = sphere.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(1, 0, 0));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sphere);
PRT_END_MAIN