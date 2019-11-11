#include "geometry/Sphere.h"
#include "geometry/CollisionData.h"
#include "geometry/Plane.h"

#include "math/Spherical.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Spherical)
PR_TEST("UV (0,0)")
{
	float u = 0, v = 0;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(u,v));
}
PR_TEST("UV (1,0)")
{
	float u = 1, v = 0;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(0,v));// Ambiguous
}
PR_TEST("UV (0,1)")
{
	float u = 0, v = 1;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(0.5,v));// Ambiguous
}
PR_TEST("UV (1,1)")
{
	float u = 1, v = 1;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(0.5,v));// Ambiguous
}
PR_TEST("UV (0.5,0.5)")
{
	float u = 0.5f, v = 0.5f;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(u,v));
}
PR_TEST("UV (0.75,0.25)")
{
	float u = 0.75f, v = 0.25f;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(u,v));
}
PR_TEST("UV (0.25,0.75)")
{
	float u = 0.25f, v = 0.75f;
	auto n = Spherical::cartesian_from_uv(u, v);

	Vector2f ruv = Spherical::uv_from_normal(n);
	PR_CHECK_NEARLY_EQ(ruv, Vector2f(u,v));
}
PR_END_TESTCASE()

PR_BEGIN_TESTCASE(Sphere)
PR_TEST("Size")
{
	Sphere sphere(1);
	PR_CHECK_NEARLY_EQ(sphere.volume(), 4 * PR_PI / 3);
}

PR_TEST("Intersects")
{
	Sphere sphere(1);
	Ray ray(Vector3f(-2, 0, 0),
			Vector3f(1, 0, 0));

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_TEST("Intersects Inside")
{
	Sphere sphere(1);
	Ray ray(Vector3f(0, 0, 0),
			Vector3f(1, 0, 0));

	SingleCollisionOutput s;
	sphere.intersects(ray, s);
	PR_CHECK_NEARLY_EQ(s.HitDistance, 1);
}

PR_TEST("Intersects [Package]")
{
	Sphere sphere(1);
	RayPackage rays(Vector3fv(simdpp::make_float(-2, -2, -2, -2),
							  simdpp::make_float(0, 0, 0, 0),
							  simdpp::make_float(0, 0, 0, 0)),
					Vector3fv(simdpp::make_float(1, 1, -1, 0),
							  simdpp::make_float(0, 0, 0, 1),
							  simdpp::make_float(0, 0, 0, 0)));

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
PRT_TESTCASE(Spherical);
PRT_TESTCASE(Sphere);
PRT_END_MAIN