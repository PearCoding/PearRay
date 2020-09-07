#include "math/Triangulation.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Triangulation2D)
PR_TEST("Simple Quad")
{
	std::vector<Vector2f> points;
	points.emplace_back(0.0f, 0.0f);
	points.emplace_back(1.0f, 0.0f);
	points.emplace_back(0.0f, 1.0f);
	points.emplace_back(1.0f, 1.0f);

	auto tri = Triangulation::triangulate2D(points);
	PR_CHECK_EQ(tri.size(), 2);
}
PR_END_TESTCASE()

PR_BEGIN_TESTCASE(Triangulation3D)
PR_TEST("Simple Cube")
{
	std::vector<Vector3f> points;
	points.emplace_back(0.0f, 0.0f, 0.0f);
	points.emplace_back(1.0f, 0.0f, 0.0f);
	points.emplace_back(0.0f, 1.0f, 0.0f);
	points.emplace_back(1.0f, 1.0f, 0.0f);
	points.emplace_back(0.0f, 0.0f, 1.0f);
	points.emplace_back(1.0f, 0.0f, 1.0f);
	points.emplace_back(0.0f, 1.0f, 1.0f);
	points.emplace_back(1.0f, 1.0f, 1.0f);

	auto tri = Triangulation::triangulate3D(points);
	PR_CHECK_EQ(tri.size(), 12);

	const auto normal = [&](const Triangulation::Triangle& t) {
		const Vector3f a = points[t.V0];
		const Vector3f b = points[t.V1];
		const Vector3f c = points[t.V2];
		return (b - a).cross(c - a).normalized();
	};

	// Check that only one axis of the normal is non-zero, else an ill-formed triangulation was constructed
	size_t xnum = 0;
	size_t ynum = 0;
	size_t znum = 0;
	for (size_t i = 0; i < tri.size(); ++i) {
		const Vector3f N = normal(tri[i]).cwiseAbs();
		bool xortho		 = (N(0) > PR_EPSILON && N(1) <= PR_EPSILON && N(2) <= PR_EPSILON);
		bool yortho		 = (N(0) <= PR_EPSILON && N(1) > PR_EPSILON && N(2) <= PR_EPSILON);
		bool zortho		 = (N(0) <= PR_EPSILON && N(1) <= PR_EPSILON && N(2) > PR_EPSILON);
		PR_CHECK_TRUE(xortho || yortho || zortho);
		if (xortho)
			++xnum;
		if (yortho)
			++ynum;
		if (zortho)
			++znum;
	}

	PR_CHECK_EQ(xnum, 4);
	PR_CHECK_EQ(ynum, 4);
	PR_CHECK_EQ(znum, 4);
}
PR_TEST("Simple Cube Filled")
{
	std::vector<Vector3f> points;
	points.emplace_back(0.0f, 0.0f, 0.0f);
	points.emplace_back(1.0f, 0.0f, 0.0f);
	points.emplace_back(0.0f, 1.0f, 0.0f);
	points.emplace_back(1.0f, 1.0f, 0.0f);
	points.emplace_back(0.0f, 0.0f, 1.0f);
	points.emplace_back(1.0f, 0.0f, 1.0f);
	points.emplace_back(0.0f, 1.0f, 1.0f);
	points.emplace_back(1.0f, 1.0f, 1.0f);

	auto tri = Triangulation::triangulate3D(points, false);
	PR_CHECK_EQ(tri.size(), 16);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Triangulation2D);
PRT_TESTCASE(Triangulation3D);
PRT_END_MAIN