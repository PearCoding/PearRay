#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Plane)
PR_TEST("Size")
{
	Plane plane(10, 10);
	PR_CHECK_EQ(plane.xAxis().norm(), 10);
	PR_CHECK_EQ(plane.yAxis().norm(), 10);
	PR_CHECK_EQ(plane.surfaceArea(), 100);
}

PR_TEST("Axis")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(10, 0, 0), Eigen::Vector3f(0, 10, 0));
	PR_CHECK_EQ(plane.xAxis().norm(), 10);
	PR_CHECK_EQ(plane.yAxis().norm(), 10);
	PR_CHECK_EQ(plane.surfaceArea(), 100);
}

PR_TEST("Normal")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 1, 0));
	Eigen::Vector3f norm = plane.normal();
	PR_CHECK_NEARLY_EQ(norm, Eigen::Vector3f(0, 0, 1));
}

PR_TEST("Normal 2")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 0, 1));
	Eigen::Vector3f norm = plane.normal();
	PR_CHECK_NEARLY_EQ(norm, Eigen::Vector3f(0, -1, 0));
}

PR_TEST("Center")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 1, 0));
	Eigen::Vector3f center = plane.center();
	PR_CHECK_NEARLY_EQ(center, Eigen::Vector3f(0.5, 0.5, 0));
}

PR_TEST("Contains")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 1, 0));
	PR_CHECK_TRUE(plane.contains(Eigen::Vector3f(0.5, 0.5, 0)));
	PR_CHECK_FALSE(plane.contains(Eigen::Vector3f(-0.5, 0.5, 0)));
	PR_CHECK_TRUE(plane.contains(Eigen::Vector3f(0.5, 0.5, 0)));
	PR_CHECK_FALSE(plane.contains(Eigen::Vector3f(-0.5, 0.5, 0)));
}

PR_TEST("Contains 2")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(10, 0, 0), Eigen::Vector3f(0, 10, 0));
	PR_CHECK_TRUE(plane.contains(Eigen::Vector3f(5, 5, 0)));
	PR_CHECK_FALSE(plane.contains(Eigen::Vector3f(-5, 5, 0)));
	PR_CHECK_TRUE(plane.contains(Eigen::Vector3f(5, 5, 0)));
	PR_CHECK_FALSE(plane.contains(Eigen::Vector3f(-5, 5, 0)));
}

PR_TEST("Intersects 1")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 1, 0));
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0.5, 0.5, -1), Eigen::Vector3f(0, 0, 1));

	Plane::Intersection s = plane.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_NEARLY_EQ(s.Position, Eigen::Vector3f(0.5, 0.5, 0));
	PR_CHECK_NEARLY_EQ(s.UV, Eigen::Vector2f(0.5, 0.5));
}

PR_TEST("Intersects 2")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(0, 1, 0));
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0.5, 0.5, -1), Eigen::Vector3f(0, 1, 0));

	Plane::Intersection s = plane.intersects(ray);
	PR_CHECK_FALSE(s.Successful);
}

PR_TEST("Intersects 3")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(10, 0, 0), Eigen::Vector3f(0, 10, 0));
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(5, 5, -1), Eigen::Vector3f(0, 0, 1));

	Plane::Intersection s = plane.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_NEARLY_EQ(s.Position, Eigen::Vector3f(5, 5, 0));
	PR_CHECK_NEARLY_EQ(s.UV, Eigen::Vector2f(0.5, 0.5));
}

PR_TEST("Intersects 4")
{
	Plane plane(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(10, 0, 0), Eigen::Vector3f(0, 20, 0));
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(5, 10, -1), Eigen::Vector3f(0, 0, 1));

	Plane::Intersection s = plane.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_NEARLY_EQ(s.Position, Eigen::Vector3f(5, 10, 0));
	PR_CHECK_NEARLY_EQ(s.UV, Eigen::Vector2f(0.5, 0.5));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Plane);
PRT_END_MAIN