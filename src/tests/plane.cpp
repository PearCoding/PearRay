#include "geometry/Plane.h"
#include "geometry/CollisionData.h"

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
	Plane plane(Vector3f(0, 0, 0), Vector3f(10, 0, 0), Vector3f(0, 10, 0));
	PR_CHECK_EQ(plane.xAxis().norm(), 10);
	PR_CHECK_EQ(plane.yAxis().norm(), 10);
	PR_CHECK_EQ(plane.surfaceArea(), 100);
}

PR_TEST("Normal")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0));
	Vector3f norm = plane.normal();
	PR_CHECK_NEARLY_EQ(norm, Vector3f(0, 0, 1));
}

PR_TEST("Normal 2")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 0, 1));
	Vector3f norm = plane.normal();
	PR_CHECK_NEARLY_EQ(norm, Vector3f(0, -1, 0));
}

PR_TEST("Center")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0));
	Vector3f center = plane.center();
	PR_CHECK_NEARLY_EQ(center, Vector3f(0.5, 0.5, 0));
}

PR_TEST("Contains")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0));
	PR_CHECK_TRUE(plane.contains(Vector3f(0.5, 0.5, 0)));
	PR_CHECK_FALSE(plane.contains(Vector3f(-0.5, 0.5, 0)));
	PR_CHECK_TRUE(plane.contains(Vector3f(0.5, 0.5, 0)));
	PR_CHECK_FALSE(plane.contains(Vector3f(-0.5, 0.5, 0)));
}

PR_TEST("Contains 2")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(10, 0, 0), Vector3f(0, 10, 0));
	PR_CHECK_TRUE(plane.contains(Vector3f(5, 5, 0)));
	PR_CHECK_FALSE(plane.contains(Vector3f(-5, 5, 0)));
	PR_CHECK_TRUE(plane.contains(Vector3f(5, 5, 0)));
	PR_CHECK_FALSE(plane.contains(Vector3f(-5, 5, 0)));
}

PR_TEST("Intersects 1")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0));

	Ray ray(Vector3f(0.5, 0.5, -1),
			Vector3f(0, 0, 1));

	SingleCollisionOutput s;
	plane.intersects(ray, s);

	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
	PR_CHECK_NEARLY_EQ(s.UV[0], 0.5);
	PR_CHECK_NEARLY_EQ(s.UV[1], 0.5);
}

PR_TEST("Intersects 2")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0));

	Ray ray(Vector3f(0.5, 0.5, -1),
			Vector3f(0, 1, 0));

	SingleCollisionOutput s;
	plane.intersects(ray, s);
	PR_CHECK_FALSE(s.HitDistance < std::numeric_limits<float>::infinity());
}

PR_TEST("Intersects 3")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(10, 0, 0), Vector3f(0, 10, 0));
	Ray ray(Vector3f(5, 5, -1),
			Vector3f(0, 0, 1));

	SingleCollisionOutput s;
	plane.intersects(ray, s);

	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
	PR_CHECK_NEARLY_EQ(s.UV[0], 0.5);
	PR_CHECK_NEARLY_EQ(s.UV[1], 0.5);
}

PR_TEST("Intersects 4")
{
	Plane plane(Vector3f(0, 0, 0), Vector3f(10, 0, 0), Vector3f(0, 20, 0));
	Ray ray(Vector3f(5, 10, -1),
			Vector3f(0, 0, 1));

	SingleCollisionOutput s;
	plane.intersects(ray, s);

	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
	PR_CHECK_NEARLY_EQ(s.UV[0], 0.5);
	PR_CHECK_NEARLY_EQ(s.UV[1], 0.5);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Plane);
PRT_END_MAIN