#include "TriMesh.h"
#include "Logger.h"
#include "Platform.h"
#include "Profiler.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "geometry/GeometryPoint.h"
#include "math/Tangent.h"
#include "sampler/SplitSample.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
TriMesh::TriMesh(const std::shared_ptr<MeshContainer>& mesh_container)
	: mContainer(mesh_container)
{
	mContainer->triangulate(); // Make sure it is only triangles!
}

TriMesh::~TriMesh()
{
}

float TriMesh::surfaceArea(uint32 id) const
{
	return mContainer->surfaceArea(id, Eigen::Affine3f());
}

bool TriMesh::isCollidable() const
{
	return mContainer->isValid() && mKDTree;
}

float TriMesh::collisionCost() const
{
	return static_cast<float>(mContainer->faceCount());
}

void TriMesh::build(const std::wstring& cnt_file)
{
	BUILDER builder(mContainer.get(), [](void* observer, size_t f) {
								MeshContainer* mesh = reinterpret_cast<MeshContainer*>(observer);
								const uint32 ind1 = mesh->indices()[3*f];
								const uint32 ind2 = mesh->indices()[3*f+1];
								const uint32 ind3 = mesh->indices()[3*f+2];

								Vector3f p1 = mesh->vertex(ind1);
								Vector3f p2 = mesh->vertex(ind2);
								Vector3f p3 = mesh->vertex(ind3);
								return Triangle::getBoundingBox(p1,p2,p3); },
					[](void*, size_t) {
						return 4.0f;
					});

	builder.build(mContainer->faceCount());

	std::ofstream stream(encodePath(cnt_file), std::ios::out | std::ios::trunc);
	builder.save(stream);

	PR_LOG(L_INFO) << "Mesh KDtree [depth="
				   << builder.depth()
				   << ", elements=" << mContainer->faceCount()
				   << ", leafs=" << builder.leafCount()
				   << ", elementsPerLeaf=[avg:" << builder.avgElementsPerLeaf()
				   << ", min:" << builder.minElementsPerLeaf()
				   << ", max:" << builder.maxElementsPerLeaf()
				   << ", ET:" << builder.expectedTraversalSteps()
				   << ", EL:" << builder.expectedLeavesVisited()
				   << ", EI:" << builder.expectedObjectsIntersected()
				   << "]]" << std::endl;

	load(cnt_file);
}

void TriMesh::load(const std::wstring& cnt_file)
{
	if (mKDTree)
		return;

	std::ifstream stream(encodePath(cnt_file));
	mKDTree = std::make_unique<kdTreeCollider>();
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

BoundingBox TriMesh::localBoundingBox() const
{
	return mKDTree ? mKDTree->boundingBox() : BoundingBox();
}

void TriMesh::checkCollision(const RayPackage& in, CollisionOutput& out) const
{
	PR_ASSERT(mKDTree, "Build before collision checks");
	mKDTree
		->checkCollision(in, out,
						 [this](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
							 Vector2fv uv;
							 vfloat t;

							 const uint32 ind1 = mContainer->indices()[3 * f];
							 const uint32 ind2 = mContainer->indices()[3 * f + 1];
							 const uint32 ind3 = mContainer->indices()[3 * f + 2];

							 const Vector3fv p0 = promote(mContainer->vertex(ind1));
							 const Vector3fv p1 = promote(mContainer->vertex(ind2));
							 const Vector3fv p2 = promote(mContainer->vertex(ind3));

							 bfloat hits = Triangle::intersect(
								 in2,
								 p0, p1, p2,
								 uv,
								 t); // Major bottleneck!

							 const vfloat inf = simdpp::make_float(std::numeric_limits<float>::infinity());
							 out2.HitDistance = simdpp::blend(t, inf, hits);

							 if (mContainer->features() & MF_HAS_UV) {
								 Vector2f uv0 = mContainer->uv(ind1);
								 Vector2f uv1 = mContainer->uv(ind2);
								 Vector2f uv2 = mContainer->uv(ind3);
								 for (int i = 0; i < 2; ++i)
									 out2.Parameter[i] = uv1(i) * uv(0)
														 + uv2(i) * uv(1)
														 + uv0(i) * (1 - uv(0) - uv(1));
							 } else {
								 out2.Parameter[0] = uv(0);
								 out2.Parameter[1] = uv(1);
							 }

							 if (mContainer->features() & MF_HAS_MATERIAL)
								 out2.MaterialID = simdpp::make_uint(mContainer->materialSlot(f)); // Has to be updated in entity!
							 else
								 out2.MaterialID = simdpp::make_uint(0);

							 out2.FaceID = simdpp::make_uint(f);
							 //out2.EntityID; Ignore
						 });
}

void TriMesh::checkCollision(const Ray& in, SingleCollisionOutput& out) const
{
	PR_ASSERT(mKDTree, "Build before collision checks");
	mKDTree
		->checkCollision(in, out,
						 [this](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
							 const uint32 ind1 = mContainer->indices()[3 * f];
							 const uint32 ind2 = mContainer->indices()[3 * f + 1];
							 const uint32 ind3 = mContainer->indices()[3 * f + 2];

							 float t;
							 Vector2f uv;
							 bool hit = Triangle::intersect(
								 in2,
								 mContainer->vertex(ind1), mContainer->vertex(ind2), mContainer->vertex(ind3),
								 uv, t); // Major bottleneck!

							 if (!hit)
								 return;
							 else
								 out2.HitDistance = t;

							 if (mContainer->features() & MF_HAS_UV) {
								 Vector2f v = Triangle::interpolate(
									 mContainer->uv(ind1), mContainer->uv(ind2), mContainer->uv(ind3),
									 uv);
								 out2.Parameter[0] = v(0);
								 out2.Parameter[1] = v(1);
							 } else {
								 out2.Parameter[0] = uv(0);
								 out2.Parameter[1] = uv(1);
							 }

							 out2.MaterialID = mContainer->materialSlot(f); // Has to be updated in entity!
							 out2.FaceID	 = static_cast<uint32>(f);		// TODO: Maybe change to 64bit?
																			//out2.EntityID; Ignore
						 });
}

Vector3f TriMesh::pickRandomParameterPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const
{
	PR_PROFILE_THIS;

	SplitSample2D split(rnd, 0, mContainer->faceCount());
	faceID	= split.integral1();
	Face face = mContainer->getFace(split.integral1());

	pdf = 1.0f / (mContainer->faceCount() * face.surfaceArea());
	return Vector3f(rnd(0), rnd(1), 0);
}

void TriMesh::provideGeometryPoint(uint32 faceID, const Vector3f& parameter, GeometryPoint& pt) const
{
	const float u = parameter[0];
	const float v = parameter[1];

	Face f = mContainer->getFace(faceID);

	Vector2f local_uv;
	if (mContainer->features() & MF_HAS_UV)
		local_uv = f.mapGlobalToLocalUV(Vector2f(u, v));
	else
		local_uv = Vector2f(u, v);

	local_uv(0) = std::min(1.0f, std::max(0.0f, local_uv(0)));
	local_uv(1) = std::min(1.0f, std::max(0.0f, local_uv(1)));

	Vector2f uv;
	f.interpolate(local_uv, pt.P, pt.N, uv);

	if (mContainer->features() & MF_HAS_UV)
		f.tangentFromUV(pt.N, pt.Nx, pt.Ny);
	else
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

	pt.UVW = Vector3f(uv(0), uv(1), 0);
}

} // namespace PR
