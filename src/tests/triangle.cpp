#include "geometry/Triangle.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Triangle)
PR_TEST("Intersects CCW")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0.5, 0.5, -1), Eigen::Vector3f(0, 0, 1));

	Eigen::Vector3f point;
	float u, v;
	float t;
	PR_CHECK_TRUE(Triangle::intersect(ray, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,0,0), Eigen::Vector3f(0,1,0),
		u, v, point, t));
	PR_CHECK_NEARLY_EQ(point, Eigen::Vector3f(0.5, 0.5, 0));
	PR_CHECK_NEARLY_EQ(u, 0.5);
	PR_CHECK_NEARLY_EQ(v, 0.5);
}

PR_TEST("Intersects CW")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0.5, 0.5, -1), Eigen::Vector3f(0, 0, 1));

	Eigen::Vector3f point;
	float u, v;
	float t;
	PR_CHECK_TRUE(Triangle::intersect(ray, Eigen::Vector3f(0,0,0), Eigen::Vector3f(0,1,0), Eigen::Vector3f(1,0,0),
		u, v, point, t));
	PR_CHECK_NEARLY_EQ(point, Eigen::Vector3f(0.5, 0.5, 0));
	PR_CHECK_NEARLY_EQ(u, 0.5);
	PR_CHECK_NEARLY_EQ(v, 0.5);
}

PR_TEST("Intersects Border")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0, 0, -1), Eigen::Vector3f(0, 0, 1));

	Eigen::Vector3f point;
	float u, v;
	float t;
	PR_CHECK_TRUE(Triangle::intersect(ray, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,0,0), Eigen::Vector3f(0,1,0),
		u, v, point, t));
	PR_CHECK_NEARLY_EQ(point, Eigen::Vector3f(0, 0, 0));
	PR_CHECK_NEARLY_EQ(u, 0);
	PR_CHECK_NEARLY_EQ(v, 0);
}

PR_TEST("Intersects Not")
{
	Ray ray(Eigen::Vector2i(0,0), Eigen::Vector3f(0.6, 0.6, -1), Eigen::Vector3f(0, 0, 1));

	Eigen::Vector3f point;
	float u, v;
	float t;
	PR_CHECK_FALSE(Triangle::intersect(ray, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,0,0), Eigen::Vector3f(0,1,0),
		u, v, point, t));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Triangle);
PRT_END_MAIN