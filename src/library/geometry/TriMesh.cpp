#include "TriMesh.h"
#include "Logger.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "math/Projection.h"

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

void TriMesh::build(const std::string& container_file, bool loadOnly)
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

void TriMesh::buildTree(const std::string& file)
{
	BUILDER builder(this, [](void* observer, uint64 f) {
								TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
								const uint32 ind1 = mesh->mIndices[0][f];
								const uint32 ind2 = mesh->mIndices[1][f];
								const uint32 ind3 = mesh->mIndices[2][f];

								Eigen::Vector3f p1 = Eigen::Vector3f(mesh->mVertices[0][ind1],
																	mesh->mVertices[1][ind1],
																	mesh->mVertices[2][ind1]);
								Eigen::Vector3f p2 = Eigen::Vector3f(mesh->mVertices[0][ind2],
																	mesh->mVertices[1][ind2],
																	mesh->mVertices[2][ind2]);
								Eigen::Vector3f p3 = Eigen::Vector3f(mesh->mVertices[0][ind3],
																	mesh->mVertices[1][ind3],
																	mesh->mVertices[2][ind3]);
								return Triangle::getBoundingBox(p1,p2,p3); },
					[](void* observer, uint64) {
						TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
						return mesh->intersectionTestCost();
					});
	builder.build(faceCount());

	std::ofstream stream(file, std::ios::out | std::ios::trunc);
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

void TriMesh::loadTree(const std::string& file)
{
	if (mKDTree) {
		delete mKDTree;
		mKDTree = nullptr;
	}

	std::ifstream stream(file);
	mKDTree = new kdTreeCollider;
	mKDTree->load(stream);
	if (!mKDTree->isEmpty())
		mBoundingBox = mKDTree->boundingBox();
}

float TriMesh::faceArea(uint32 f, const Eigen::Affine3f& transform) const
{
	const uint32 ind1 = mIndices[0][f];
	const uint32 ind2 = mIndices[1][f];
	const uint32 ind3 = mIndices[2][f];

	float p1x, p1y, p1z;
	transformV(transform.matrix(),
			   mVertices[0][ind1], mVertices[1][ind1], mVertices[2][ind1],
			   p1x, p1y, p1z);

	float p2x, p2y, p2z;
	transformV(transform.matrix(),
			   mVertices[0][ind2], mVertices[1][ind2], mVertices[2][ind2],
			   p2x, p2y, p2z);

	float p3x, p3y, p3z;
	transformV(transform.matrix(),
			   mVertices[0][ind3], mVertices[1][ind3], mVertices[2][ind3],
			   p3x, p3y, p3z);

	return Triangle::surfaceArea(p1x, p1y, p1z,
								 p2x, p2y, p2z,
								 p3x, p3y, p3z);
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

bool TriMesh::checkCollision(const CollisionInput& in, CollisionOutput& out) const
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	return mKDTree
		->checkCollision(in, out,
						 [this](const CollisionInput& in2, uint64 f, CollisionOutput& out2) {
							 vfloat u, v, t;

							 const uint32 i1 = mIndices[0][f];
							 const uint32 i2 = mIndices[1][f];
							 const uint32 i3 = mIndices[2][f];

							 bfloat hits = Triangle::intersect(in2,
															   mVertices[0][i1], mVertices[1][i1], mVertices[2][i1],
															   mVertices[0][i2], mVertices[1][i2], mVertices[2][i2],
															   mVertices[0][i3], mVertices[1][i3], mVertices[2][i3],
															   u, v,
															   t); // Major bottleneck!

							 const vfloat inf = simdpp::make_float(std::numeric_limits<float>::infinity());
							 out2.HitDistance = simdpp::blend(t, inf, hits);

							 if (features() & TMF_HAS_UV) {
								 for (int i = 0; i < 2; ++i)
									 out2.UV[i] = mUVs[i][i2] * u
												  + mUVs[i][i3] * v
												  + mUVs[i][i1] * (1 - u - v);
							 } else {
								 out2.UV[0] = u;
								 out2.UV[1] = v;
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
	return faceCount();
}

void TriMesh::sampleFacePoint(const vfloat& rnd1, const vfloat& rnd2, const vfloat& rnd3,
							  ShadingPoint& p, vfloat& pdfA) const
{
	vuint32 fi = simdpp::to_uint32(rnd1 * (faceCount() - 1));
	vfloat b1, b2;
	Projection::triangleV(rnd2, rnd3, b1, b2);

	FacePackage pkg = getFaces(fi);
	pkg.interpolate(b1, b2,
					p.P[0], p.P[1], p.P[2],
					p.Ng[0], p.Ng[1], p.Ng[2],
					p.UVW[0], p.UVW[1]);
	p.MaterialID = pkg.MaterialSlot;

	// TODO:
	//pdfA = 1.0f / (mIndices.size() * Triangle::surfaceArea(face.V[0], face.V[1], face.V[2]));
}
} // namespace PR
