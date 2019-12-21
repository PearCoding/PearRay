#include "geometry/CollisionData.h"
#include "math/SIMD.h"
#include "mesh/TriMesh.h"

#include "Test.h"

using namespace PR;

template <typename T>
inline void addVertex(T p1, T p2, T p3,
					  std::vector<T>& c)
{
	c.emplace_back(p1);
	c.emplace_back(p2);
	c.emplace_back(p3);
}

template <typename T>
inline void addVertex(T p1, T p2,
					  std::vector<T>& c)
{
	c.emplace_back(p1);
	c.emplace_back(p2);
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
	std::vector<float> vertices;
	addVertex<float>(-2, 0, 0, vertices);
	addVertex<float>(-1, 1, 0, vertices);
	addVertex<float>(-1, 0, 0, vertices);

	addVertex<float>(1, 0, 0, vertices);
	addVertex<float>(1, 1, 0, vertices);
	addVertex<float>(2, 1, 0, vertices);

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices);
	cnt->setNormals(vertices); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_EQ(cnt->nodeCount(), 6);
	PR_CHECK_EQ(cnt->faceCount(), 2);
	PR_CHECK_EQ(cnt->triangleCount(), 2);
	PR_CHECK_EQ(cnt->quadCount(), 0);

	TriMesh mesh(cnt);
	mesh.build(L"tmp1.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(-1.5f, 1.5f, 0, 0.6f);
	in.Origin[1] = simdpp::make_float(0.5f, 0.5f, 0.5f, 0.6f);
	in.Origin[2] = simdpp::make_float(1, 1, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

	in.cache();

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
	std::vector<float> vertices;
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
	cnt->setVertices(vertices);
	cnt->setNormals(vertices); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5, 6, 7, 8 });
	cnt->setFaceVertexCount({ 3, 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_EQ(cnt->nodeCount(), 9);
	PR_CHECK_EQ(cnt->faceCount(), 3);
	PR_CHECK_EQ(cnt->triangleCount(), 3);
	PR_CHECK_EQ(cnt->quadCount(), 0);

	TriMesh mesh(cnt);
	mesh.build(L"tmp2.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75f, 0.75f, 0, 5);
	in.Origin[1] = simdpp::make_float(0.5f, 0.5f, 0.5f, 5);
	in.Origin[2] = simdpp::make_float(2, -2, 1, 1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1, 1, 1, 1);

	in.cache();

	CollisionOutput out;
	mesh.checkCollision(in, out);

	// From top to bottom
	// FIXME: There is a bug in the parallel tracing!
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
	std::vector<float> vertices;
	addVertex<float>(0, 0, -1, vertices);
	addVertex<float>(1, 0, -1, vertices);
	addVertex<float>(1, 1, 0, vertices);

	addVertex<float>(0, 0, 0, vertices);
	addVertex<float>(1, 0, 1, vertices);
	addVertex<float>(1, 1, 1, vertices);

	std::vector<float> uvs;
	addVertex<float>(0.1f, 0.1f, uvs);
	addVertex<float>(0.2f, 0.2f, uvs);
	addVertex<float>(0.3f, 0.3f, uvs);

	addVertex<float>(0.4f, 0.4f, uvs);
	addVertex<float>(0.5f, 0.5f, uvs);
	addVertex<float>(0.6f, 0.6f, uvs);

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices);
	cnt->setNormals(vertices); // Bad normals, but we do not care
	cnt->setUVs(uvs);
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_TRUE(cnt->features() & MF_HAS_UV);
	PR_CHECK_EQ(cnt->nodeCount(), 6);
	PR_CHECK_EQ(cnt->faceCount(), 2);
	PR_CHECK_EQ(cnt->triangleCount(), 2);
	PR_CHECK_EQ(cnt->quadCount(), 0);

	TriMesh mesh(cnt);
	mesh.build(L"tmp3.cnt");

	RayPackage in;
	in.Origin[0] = simdpp::make_float(0.75f, 0.75f, 0, 0.6f);
	in.Origin[1] = simdpp::make_float(0.5f, 0.5f, 0.5f, 0.6f);
	in.Origin[2] = simdpp::make_float(2, -2, -1, -1);

	in.Direction[0] = simdpp::make_float(0);
	in.Direction[1] = simdpp::make_float(0);
	in.Direction[2] = simdpp::make_float(-1);

	in.cache();

	CollisionOutput out;
	mesh.checkCollision(in, out);

	// From top to bottom
	PR_CHECK_TRUE(extract<0>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(extract<0>(out.FaceID), 1);
	PR_CHECK_GREAT(extract<0>(out.Parameter[0]), 0);
	PR_CHECK_GREAT(extract<0>(out.Parameter[1]), 0);

	PR_CHECK_FALSE(extract<1>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<2>(out.HitDistance) < std::numeric_limits<float>::infinity());
	PR_CHECK_FALSE(extract<3>(out.HitDistance) < std::numeric_limits<float>::infinity());
}

PR_TEST("Single Intersection")
{
	std::vector<float> vertices;
	addVertex<float>(-2, 0, 0, vertices);
	addVertex<float>(-1, 1, 0, vertices);
	addVertex<float>(-1, 0, 0, vertices);

	addVertex<float>(1, 0, 0, vertices);
	addVertex<float>(1, 1, 0, vertices);
	addVertex<float>(2, 1, 0, vertices);

	std::shared_ptr<MeshContainer> cnt = std::make_shared<MeshContainer>();
	cnt->setVertices(vertices);
	cnt->setNormals(vertices); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5 });
	cnt->setFaceVertexCount({ 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_EQ(cnt->nodeCount(), 6);
	PR_CHECK_EQ(cnt->faceCount(), 2);
	PR_CHECK_EQ(cnt->triangleCount(), 2);
	PR_CHECK_EQ(cnt->quadCount(), 0);

	TriMesh mesh(cnt);
	mesh.build(L"tmp1.cnt");

	Ray in;
	in.Origin[0] = -1.5;
	in.Origin[1] = 0.5;
	in.Origin[2] = 1;

	in.Direction[0] = 0;
	in.Direction[1] = 0;
	in.Direction[2] = -1;
	in.cache();

	SingleCollisionOutput out;
	mesh.checkCollision(in, out);

	// Left triangle
	PR_CHECK_TRUE(out.HitDistance < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(out.FaceID, 0);
}

PR_TEST("Single Intersection Overlap")
{
	std::vector<float> vertices;
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
	cnt->setVertices(vertices);
	cnt->setNormals(vertices); // Bad normals, but we do not care
	cnt->setIndices({ 0, 1, 2, 3, 4, 5, 6, 7, 8 });
	cnt->setFaceVertexCount({ 3, 3, 3 });

	PR_CHECK_TRUE(cnt->isValid());
	PR_CHECK_EQ(cnt->nodeCount(), 9);
	PR_CHECK_EQ(cnt->faceCount(), 3);
	PR_CHECK_EQ(cnt->triangleCount(), 3);
	PR_CHECK_EQ(cnt->quadCount(), 0);

	TriMesh mesh(cnt);
	mesh.build(L"tmp2.cnt");

	Ray in;
	in.Origin	= Vector3f(0.75f, 0.5f, 2);
	in.Direction = Vector3f(0, 0, -1);
	in.cache();

	SingleCollisionOutput out;
	mesh.checkCollision(in, out);

	PR_CHECK_TRUE(out.HitDistance < std::numeric_limits<float>::infinity());
	PR_CHECK_EQ(out.FaceID, 1);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(KDTree);
PRT_END_MAIN