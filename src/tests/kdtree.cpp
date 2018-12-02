#include "geometry/TriMesh.h"
#include "math/SIMD.h"

#include "Test.h"

using namespace PR;

// Make sure the tree builder gives each triangle his own volume
constexpr float CUSTOM_INTERSECTION_TEST_COST = 10000000;

template <typename T>
inline void addVertex(T p1, T p2, T p3,
					  std::vector<T>* c)
{
	c[0].emplace_back(p1);
	c[1].emplace_back(p2);
	c[2].emplace_back(p3);
}

template <typename T>
inline void addVertex(T p1, T p2,
					  std::vector<T>* c)
{
	c[0].emplace_back(p1);
	c[1].emplace_back(p2);
}

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
	std::vector<float> vertices[3];
	addVertex<float>(-2, 0, 0, vertices);
	addVertex<float>(-1, 1, 0, vertices);
	addVertex<float>(-1, 0, 0, vertices);

	addVertex<float>(1, 0, 0, vertices);
	addVertex<float>(1, 1, 0, vertices);
	addVertex<float>(2, 1, 0, vertices);

	std::vector<uint32> faces[3];
	addVertex<uint32>(0, 1, 2, faces);
	addVertex<uint32>(3, 4, 5, faces);

	TriMesh mesh;
	mesh.setVertices(vertices[0], vertices[1], vertices[2]);
	mesh.setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	mesh.setIndices(faces[0], faces[1], faces[2]);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp1.cnt", false);

	RayPackage in;
	in.Origin[0] = simdpp::make_float(-1.5, 1.5, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 0.6);
	in.Origin[2] = simdpp::make_float(1, 1, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

	in.setupInverse();

	CollisionOutput out;
	mesh.checkCollision(in, out);

	// Left triangle
	PR_CHECK_TRUE(extract<0>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<0>(out.FaceID), 0);

	// Right triangle
	PR_CHECK_TRUE(extract<1>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<1>(out.FaceID), 1);

	// Empty mid space
	PR_CHECK_FALSE(extract<2>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<3>(out.HitDistance) < std::numeric_limits<float>::infinity());
}

PR_TEST("Overlap")
{
	std::vector<float> vertices[3];
	addVertex<float>(0, 0, -1, vertices);
	addVertex<float>(1, 0, -1, vertices);
	addVertex<float>(1, 1, 0, vertices);

	addVertex<float>(0, 0, 0, vertices);
	addVertex<float>(1, 0, 1, vertices);
	addVertex<float>(1, 1, 1, vertices);

	addVertex<float>(0, 0, 3, vertices);
	addVertex<float>(1, 0, 3, vertices);
	addVertex<float>(1, 1, 2, vertices);

	std::vector<uint32> faces[3];
	addVertex<uint32>(0, 1, 2, faces);
	addVertex<uint32>(3, 4, 5, faces);
	addVertex<uint32>(6, 7, 8, faces);

	TriMesh mesh;
	mesh.setVertices(vertices[0], vertices[1], vertices[2]);
	mesh.setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	mesh.setIndices(faces[0], faces[1], faces[2]);

	PR_CHECK_EQ(mesh.nodeCount(), 9);
	PR_CHECK_EQ(mesh.faceCount(), 3);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp2.cnt", false);

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75, 0.75, 0, 5);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 5);
	in.Origin[2] = simdpp::make_float(2, -2, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1, 1, 1, 1);

	in.setupInverse();

	CollisionOutput out;
	mesh.checkCollision(in, out);

	// From top to bottom
	PR_CHECK_TRUE(extract<0>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<0>(out.FaceID), 1);

	// From bottom to top
	PR_CHECK_TRUE(extract<1>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<1>(out.FaceID), 0);

	PR_CHECK_FALSE(extract<2>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<3>(out.HitDistance) < std::numeric_limits<float>::infinity());
}

PR_TEST("UV")
{
	std::vector<float> vertices[3];
	addVertex<float>(0, 0, -1, vertices);
	addVertex<float>(1, 0, -1, vertices);
	addVertex<float>(1, 1, 0, vertices);

	addVertex<float>(0, 0, 0, vertices);
	addVertex<float>(1, 0, 1, vertices);
	addVertex<float>(1, 1, 1, vertices);

	std::vector<float> uvs[2];
	addVertex<float>(0.1, 0.1, uvs);
	addVertex<float>(0.2, 0.2, uvs);
	addVertex<float>(0.3, 0.3, uvs);

	addVertex<float>(0.4, 0.4, uvs);
	addVertex<float>(0.5, 0.5, uvs);
	addVertex<float>(0.6, 0.6, uvs);

	std::vector<uint32> faces[3];
	addVertex<uint32>(0, 1, 2, faces);
	addVertex<uint32>(3, 4, 5, faces);

	TriMesh mesh;
	mesh.setVertices(vertices[0], vertices[1], vertices[2]);
	mesh.setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	mesh.setUVs(uvs[0], uvs[1]);
	mesh.setIndices(faces[0], faces[1], faces[2]);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp3.cnt", false);

	PR_CHECK_TRUE(mesh.features() & TMF_HAS_UV);

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75, 0.75, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 0.6);
	in.Origin[2] = simdpp::make_float(2, -2, -1, -1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

	in.setupInverse();

	CollisionOutput out;
	mesh.checkCollision(in, out);

	// From top to bottom
	PR_CHECK_TRUE(extract<0>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<0>(out.FaceID), 1);
	PR_CHECK_GREAT(extract<0>(out.UV[0]), 0);
	PR_CHECK_GREAT(extract<0>(out.UV[1]), 0);

	PR_CHECK_FALSE(extract<1>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<2>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<3>(out.HitDistance) < std::numeric_limits<float>::infinity());
}

PR_TEST("Single Intersection")
{
	std::vector<float> vertices[3];
	addVertex<float>(-2, 0, 0, vertices);
	addVertex<float>(-1, 1, 0, vertices);
	addVertex<float>(-1, 0, 0, vertices);

	addVertex<float>(1, 0, 0, vertices);
	addVertex<float>(1, 1, 0, vertices);
	addVertex<float>(2, 1, 0, vertices);

	std::vector<uint32> faces[3];
	addVertex<uint32>(0, 1, 2, faces);
	addVertex<uint32>(3, 4, 5, faces);

	TriMesh mesh;
	mesh.setVertices(vertices[0], vertices[1], vertices[2]);
	mesh.setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	mesh.setIndices(faces[0], faces[1], faces[2]);

	PR_CHECK_TRUE(mesh.isValid());

	mesh.setIntersectionTestCost(CUSTOM_INTERSECTION_TEST_COST);
	mesh.build("tmp1.cnt", false);

	Ray in;
	in.Origin[0] = -1.5;
	in.Origin[1] = 0.5;
	in.Origin[2] = 1;

	in.Direction[0] = 0;
	in.Direction[1] = 0;
	in.Direction[2] = -1;

	in.setupInverse();

	SingleCollisionOutput out;
	mesh.checkCollision(in, out);

	// Left triangle
	PR_CHECK_TRUE(out.HitDistance < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(out.FaceID, 0);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(KDTree);
PRT_END_MAIN