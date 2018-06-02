#include "geometry/TriMesh.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

// Make sure the tree builder gives each triangle his own volume
constexpr float CUSTOM_INTERSECTION_TEST_COST = 10000000;

PR_BEGIN_TESTCASE(KDTree)
/*
 *
 * 1       x     x----x
 *       / |     |   /
 *      /  |     |  /
 *     /   |     | /
 * 0  x----x     x
 * 
 *   -2   -1     1    2
 */
PR_TEST("Two Half")
{
	std::vector<Vector3f> vertices;
	vertices.emplace_back(-2,0,0);
	vertices.emplace_back(-1,1,0);
	vertices.emplace_back(-1,0,0);

	vertices.emplace_back(1,0,0);
	vertices.emplace_back(1,1,0);
	vertices.emplace_back(2,1,0);

	std::vector<Vector3u64> faces;
	faces.emplace_back(0,1,2);
	faces.emplace_back(3,4,5);

	TriMesh mesh;
	mesh.setVertices(vertices);
	mesh.setNormals(vertices);// Bad normals, but we do not care
	mesh.setIndices(faces);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp1.cnt", false);

	// Left triangle
	Ray ray1(Eigen::Vector2i(0,0), Eigen::Vector3f(-1.5, 0.5, 1), Eigen::Vector3f(0, 0, -1));
	const auto res1 = mesh.checkCollision(ray1);

	PR_CHECK_TRUE(res1.Successful);
	PR_CHECK_EQ(res1.Index, 0);

	// Right triangle
	Ray ray2(Eigen::Vector2i(0,0), Eigen::Vector3f(1.5, 0.5, 1), Eigen::Vector3f(0, 0, -1));
	const auto res2 = mesh.checkCollision(ray2);

	PR_CHECK_TRUE(res2.Successful);
	PR_CHECK_EQ(res2.Index, 1);

	// Empty mid space
	Ray ray3(Eigen::Vector2i(0,0), Eigen::Vector3f(0, 0.5, 1), Eigen::Vector3f(0, 0, -1));
	const auto res3 = mesh.checkCollision(ray3);

	PR_CHECK_FALSE(res3.Successful);
}

PR_TEST("Overlap")
{
	std::vector<Vector3f> vertices;
	vertices.emplace_back(0,0,-1);
	vertices.emplace_back(1,0,-1);
	vertices.emplace_back(1,1,-0);

	vertices.emplace_back(0,0,0);
	vertices.emplace_back(1,0,1);
	vertices.emplace_back(1,1,1);

	vertices.emplace_back(0,0,3);
	vertices.emplace_back(1,0,3);
	vertices.emplace_back(1,1,2);

	std::vector<Vector3u64> faces;
	faces.emplace_back(0,1,2);
	faces.emplace_back(3,4,5);
	faces.emplace_back(6,7,8);

	TriMesh mesh;
	mesh.setVertices(vertices);
	mesh.setNormals(vertices);// Bad normals, but we do not care
	mesh.setIndices(faces);

	PR_CHECK_EQ(mesh.nodeCount(), 9);
	PR_CHECK_EQ(mesh.faceCount(), 3);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp2.cnt", false);

	// From top to bottom
	Ray ray1(Eigen::Vector2i(0,0), Eigen::Vector3f(0.75, 0.5, 2), Eigen::Vector3f(0, 0, -1));
	const auto res1 = mesh.checkCollision(ray1);

	PR_CHECK_TRUE(res1.Successful);
	PR_CHECK_EQ(res1.Index, 1);

	// From bottom to top
	Ray ray2(Eigen::Vector2i(0,0), Eigen::Vector3f(0.75, 0.5, -2), Eigen::Vector3f(0, 0, 1));
	const auto res2 = mesh.checkCollision(ray2);

	PR_CHECK_TRUE(res2.Successful);
	PR_CHECK_EQ(res2.Index, 0);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(KDTree);
PRT_END_MAIN