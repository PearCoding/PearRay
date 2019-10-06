#include "geometry/Sphere.h"
#include "geometry/Plane.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Sphere)
PR_TEST("Size")
{
	Sphere sphere(Vector3f(0, 0, 0), 1);
	PR_CHECK_NEARLY_EQ(sphere.volume(), 4 * PR_PI / 3);
}

PR_TEST("Intersects")
{
	Sphere sphere(Vector3f(0, 0, 0), 1);
	Ray ray(Vector3f(-2, 0, 0),
			Vector3f(1, 0, 0));

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_TEST("Intersects Inside")
{
	Sphere sphere(Vector3f(0, 0, 0), 1);
	Ray ray(Vector3f(0, 0, 0),
			Vector3f(1, 0, 0));

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_TEST("Intersects [Package]")
{
	Sphere sphere(Vector3f(0, 0, 0), 1);
	RayPackage rays(Vector3fv(simdpp::make_float(-2, -2, -2, -2),
							  simdpp::make_float(0, 0, 0, 0),
							  simdpp::make_float(0, 0, 0, 0)),
					Vector3fv(simdpp::make_float(1, 1, -1, 0),
							  simdpp::make_float(0, 0, 0, 1),
							  simdpp::make_float(0, 0, 0, 0)));

	rays.setupInverse();

	CollisionOutput s;
	sphere.intersects(rays, s);
	const float INF = std::numeric_limits<float>::infinity();

	PR_CHECK_NEARLY_EQ(extract<0>(s.HitDistance), 1);
	PR_CHECK_NEARLY_EQ(extract<1>(s.HitDistance), 1);
	PR_CHECK_EQ(extract<2>(s.HitDistance), INF);
	PR_CHECK_EQ(extract<3>(s.HitDistance), INF);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Sphere);
PRT_END_MAIN