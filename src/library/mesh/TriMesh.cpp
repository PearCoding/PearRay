#include "TriMesh.h"
#include "Logger.h"
#include "Platform.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "sampler/SplitSample.h"

#include <fstream>

#define BUILDER kdTreeBuilder

namespace PR {
TriMesh::TriMesh()
	: mKDTree(nullptr)
	, mIntersectionTestCost(4.0f)
{
}

TriMesh::~TriMesh()
{
	clear();
}

void TriMesh::clear()
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}
}

void TriMesh::build(const std::wstring& container_file, bool loadOnly)
{
	PR_ASSERT(isValid(), "Mesh has to be valid before build()!");

	// Build internal KDtree
	if (!loadOnly)
		buildTree(container_file);

	loadTree(container_file);

	if (mVertices[0].size() != mVertices[1].size()
		|| mVertices[1].size() != mVertices[2].size())
		PR_LOG(L_ERROR) << "Vertex dimensional size mismatch!" << std::endl;

	if (mIndices[0].size() != mIndices[1].size()
		|| mIndices[1].size() != mIndices[2].size())
		PR_LOG(L_ERROR) << "Index dimensional size mismatch!" << std::endl;

	// Setup features
	mFeatures = 0;

	if (mUVs[0].size() == mVertices[0].size() && mUVs[1].size() == mVertices[0].size()) {
		mFeatures |= TMF_HAS_UV;
	} else if (!mUVs[0].empty() || !mUVs[1].empty()) {
		PR_LOG(L_WARNING) << "Insufficient UV data!" << std::endl;
	}

	if (mIndices[0].size() == mMaterials.size()) {
		mFeatures |= TMF_HAS_MATERIAL;
	} else if (!mMaterials.empty()) {
		PR_LOG(L_WARNING) << "Insufficient material data!" << std::endl;
	}
}

void TriMesh::buildTree(const std::wstring& file)
{
	BUILDER builder(this, [](void* observer, size_t f) {
								TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
								const uint32 ind1 = mesh->mIndices[0][f];
								const uint32 ind2 = mesh->mIndices[1][f];
								const uint32 ind3 = mesh->mIndices[2][f];

								Vector3f p1 = Vector3f(mesh->mVertices[0][ind1],
																	mesh->mVertices[1][ind1],
																	mesh->mVertices[2][ind1]);
								Vector3f p2 = Vector3f(mesh->mVertices[0][ind2],
																	mesh->mVertices[1][ind2],
																	mesh->mVertices[2][ind2]);
								Vector3f p3 = Vector3f(mesh->mVertices[0][ind3],
																	mesh->mVertices[1][ind3],
																	mesh->mVertices[2][ind3]);
								return Triangle::getBoundingBox(p1,p2,p3); },
					[](void* observer, size_t) {
						TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
						return mesh->intersectionTestCost();
					});
	builder.build(faceCount());

	std::ofstream stream(encodePath(file), std::ios::out | std::ios::trunc);
	builder.save(stream);

	PR_LOG(L_INFO) << "Mesh KDtree [depth="
				   << builder.depth()
				   << ", elements=" << faceCount()
				   << ", leafs=" << builder.leafCount()
				   << ", elementsPerLeaf=[avg:" << builder.avgElementsPerLeaf()
				   << ", min:" << builder.minElementsPerLeaf()
				   << ", max:" << builder.maxElementsPerLeaf()
				   << ", ET:" << builder.expectedTraversalSteps()
				   << ", EL:" << builder.expectedLeavesVisited()
				   << ", EI:" << builder.expectedObjectsIntersected()
				   << "]]" << std::endl;
}

void TriMesh::loadTree(const std::wstring& file)
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}

	std::ifstream stream(encodePath(file));
	mKDTree = new kdTreeCollider;
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

float TriMesh::faceArea(size_t f, const Eigen::Affine3f& tm) const
{
	const uint32 ind1 = mIndices[0][f];
	const uint32 ind2 = mIndices[1][f];
	const uint32 ind3 = mIndices[2][f];

	const Vector3f p1 = Transform::apply(tm.matrix(),
										 Vector3f(mVertices[0][ind1],
												  mVertices[1][ind1],
												  mVertices[2][ind1]));

	const Vector3f p2 = Transform::apply(tm.matrix(),
										 Vector3f(mVertices[0][ind2],
												  mVertices[1][ind2],
												  mVertices[2][ind2]));

	const Vector3f p3 = Transform::apply(tm.matrix(),
										 Vector3f(mVertices[0][ind3],
												  mVertices[1][ind3],
												  mVertices[2][ind3]));

	return Triangle::surfaceArea(p1, p2, p3);
}

float TriMesh::surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const
{
	if (!(features() & TMF_HAS_MATERIAL)) {
		if (slot == 0)
			return surfaceArea(transform);
		else
			return 0;
	}

	float a		   = 0;
	size_t counter = 0;
	for (uint32 mat : mMaterials) {
		if (mat == slot) {
			a += faceArea(counter, transform);
		}

		++counter;
	}
	return a;
}

float TriMesh::surfaceArea(const Eigen::Affine3f& transform) const
{
	float a = 0;
	for (uint32 counter = 0; counter < faceCount(); ++counter) {
		a += faceArea(counter, transform);
	}
	return a;
}

bool TriMesh::checkCollision(const Ray& in, SingleCollisionOutput& out) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	return mKDTree
		->checkCollision(in, out,
						 [this](const Ray& in2, uint64 f, SingleCollisionOutput& out2) {
							 const uint32 i1 = mIndices[0][f];
							 const uint32 i2 = mIndices[1][f];
							 const uint32 i3 = mIndices[2][f];

							 float t;
							 Vector2f uv;
							 bool hit = Triangle::intersect(
								 in2,
								 Vector3f(mVertices[0][i1], mVertices[1][i1], mVertices[2][i1]),
								 Vector3f(mVertices[0][i2], mVertices[1][i2], mVertices[2][i2]),
								 Vector3f(mVertices[0][i3], mVertices[1][i3], mVertices[2][i3]),
								 uv, t); // Major bottleneck!

							 if (!hit)
								 return;
							 else
								 out2.HitDistance = t;

							 if (features() & TMF_HAS_UV) {
								 for (int i = 0; i < 2; ++i)
									 out2.UV[i] = mUVs[i][i2] * uv(0)
												  + mUVs[i][i3] * uv(1)
												  + mUVs[i][i1] * (1 - uv(0) - uv(1));
							 } else {
								 out2.UV[0] = uv(0);
								 out2.UV[1] = uv(1);
							 }

							 if (features() & TMF_HAS_MATERIAL)
								 out2.MaterialID = mMaterials[f]; // Has to be updated in entity!
							 else
								 out2.MaterialID = 0;

							 out2.FaceID = f;
							 //out2.EntityID; Ignore
						 });
}

bool TriMesh::checkCollision(const RayPackage& in, CollisionOutput& out) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	return mKDTree
		->checkCollision(in, out,
						 [this](const RayPackage& in2, uint64 f, CollisionOutput& out2) {
							 Vector2fv uv;
							 vfloat t;

							 const uint32 i1 = mIndices[0][f];
							 const uint32 i2 = mIndices[1][f];
							 const uint32 i3 = mIndices[2][f];

							 const Vector3fv p0 = promote(
								 Vector3f(mVertices[0][i1],
										  mVertices[1][i1],
										  mVertices[2][i1]));
							 const Vector3fv p1 = promote(
								 Vector3f(mVertices[0][i2],
										  mVertices[1][i2],
										  mVertices[2][i2]));
							 const Vector3fv p2 = promote(
								 Vector3f(mVertices[0][i3],
										  mVertices[1][i3],
										  mVertices[2][i3]));

							 bfloat hits = Triangle::intersect(
								 in2,
								 p0, p1, p2,
								 uv,
								 t); // Major bottleneck!

							 const vfloat inf = simdpp::make_float(std::numeric_limits<float>::infinity());
							 out2.HitDistance = simdpp::blend(t, inf, hits);

							 if (features() & TMF_HAS_UV) {
								 for (int i = 0; i < 2; ++i)
									 out2.UV[i] = mUVs[i][i2] * uv(0)
												  + mUVs[i][i3] * uv(1)
												  + mUVs[i][i1] * (1 - uv(0) - uv(1));
							 } else {
								 out2.UV[0] = uv(0);
								 out2.UV[1] = uv(1);
							 }

							 if (features() & TMF_HAS_MATERIAL)
								 out2.MaterialID = simdpp::make_uint(mMaterials[f]); // Has to be updated in entity!
							 else
								 out2.MaterialID = simdpp::make_uint(0);

							 out2.FaceID = simdpp::make_uint(f);
							 //out2.EntityID; Ignore
						 });
}

float TriMesh::collisionCost() const
{
	return static_cast<float>(faceCount());
}

void TriMesh::sampleFacePoint(const Vector2f& rnd,
							  GeometryPoint& p, float& pdfA) const
{
	SplitSample2D smp(rnd, 0, (uint32)faceCount());

	uint32 fi  = smp.integral1();
	Vector2f b = Projection::triangle(smp.uniform1(), smp.uniform2());

	Face f = getFace(fi);

	Vector3f po, ng;
	Vector2f uv;
	f.interpolate(b, po, ng, uv);

	p.P			 = po;
	p.N			 = ng;
	p.UVW		 = Vector3f(uv(0), uv(1), 0);
	p.MaterialID = f.MaterialSlot;

	pdfA = 1.0f / (mIndices[0].size() * Triangle::surfaceArea(f.V[0], f.V[1], f.V[2]));
}

void TriMesh::provideGeometryPoint(uint32 faceID, float u, float v,
								   GeometryPoint& pt) const
{
	Face f = getFace(faceID);

	Vector2f local_uv;
	if (features() & TMF_HAS_UV)
		local_uv = f.mapGlobalToLocalUV(Vector2f(u, v));
	else
		local_uv = Vector2f(u, v);

	Vector2f uv;
	f.interpolate(local_uv, pt.P, pt.N, uv);

	if (features() & TMF_HAS_UV)
		f.tangentFromUV(pt.N, pt.Nx, pt.Ny);
	else
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

	pt.UVW		  = Vector3f(uv(0), uv(1), 0);
	pt.MaterialID = f.MaterialSlot;
}
} // namespace PR
