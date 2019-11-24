#include "geometry/CollisionData.h"
#include "math/SIMD.h"
#include "mesh/TriMesh.h"

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

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices[0], vertices[1], vertices[2]);
	cnt->setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());

	TriMesh mesh(cnt);
	mesh.build(L"tmp1.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(-1.5, 1.5, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 0.6);
	in.Origin[2] = simdpp::make_float(1, 1, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

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

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices[0], vertices[1], vertices[2]);
	cnt->setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5, 6, 7, 8 });
	cnt->setFaceVertexCount({ 3, 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());

	PR_CHECK_EQ(cnt->nodeCount(), 9);
	PR_CHECK_EQ(cnt->faceCount(), 3);

	TriMesh mesh(cnt);
	mesh.build(L"tmp2.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75, 0.75, 0, 5);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 5);
	in.Origin[2] = simdpp::make_float(2, -2, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1, 1, 1, 1);

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
	addVertex<float>(0.1f, 0.1f, uvs);
	addVertex<float>(0.2f, 0.2f, uvs);
	addVertex<float>(0.3f, 0.3f, uvs);

	addVertex<float>(0.4f, 0.4f, uvs);
	addVertex<float>(0.5f, 0.5f, uvs);
	addVertex<float>(0.6f, 0.6f, uvs);

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices[0], vertices[1], vertices[2]);
	cnt->setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	cnt->setUVs(uvs[0], uvs[1]);
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_TRUE(cnt->features() & MF_HAS_UV);

	TriMesh mesh(cnt);
	mesh.build(L"tmp3.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75, 0.75, 0, 0.6);
	in.Origin[1] = simdpp::make_float(0.5, 0.5, 0.5, 0.6);
	in.Origin[2] = simdpp::make_float(2, -2, -1, -1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

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

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices[0], vertices[1], vertices[2]);
	cnt->setNormals(vertices[0], vertices[1], vertices[2]); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });
	PR_CHECK_TRUE(cnt->isValid());

	TriMesh mesh(cnt);
	mesh.build(L"tmp1.cnt");

	Ray in;
	in.Origin[0] = -1.5;
	in.Origin[1] = 0.5;
	in.Origin[2] = 1;

	in.Direction[0] = 0;
	in.Direction[1] = 0;
	in.Direction[2] = -1;

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