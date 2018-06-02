#include "TriMesh.h"
#include "Logger.h"
#include "container/kdTreeBuilder.h"
#include "container/kdTreeBuilderNaive.h"
#include "container/kdTreeCollider.h"
#include "material/Material.h"
#include "math/Projection.h"

#include <fstream>

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

	// Setup features
	mFeatures = 0;
	if (mUVs.size() == mVertices.size()) {
		mFeatures |= TMF_HAS_UV;
	}
	if (mVelocities.size() == mVertices.size()) {
		mFeatures |= TMF_HAS_VELOCITY;
	}
	if (mIndices.size() == mMaterials.size()) {
		mFeatures |= TMF_HAS_MATERIAL;
	}
}

void TriMesh::buildTree(const std::string& file)
{
	kdTreeBuilderNaive builder(this, [](void* observer, uint64 f) {
								TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
								return Triangle::getBoundingBox(
									mesh->mVertices[mesh->mIndices[f][0]],
																mesh->mVertices[mesh->mIndices[f][1]],
																mesh->mVertices[mesh->mIndices[f][2]]); },
							   [](void* observer, uint64) {
								   TriMesh* mesh = reinterpret_cast<TriMesh*>(observer);
								   return mesh->intersectionTestCost();
							   });
	builder.build(mIndices.size());

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
			a += Triangle::surfaceArea(
				transform * (Eigen::Vector3f)mVertices[mIndices[counter][0]],
				transform * (Eigen::Vector3f)mVertices[mIndices[counter][1]],
				transform * (Eigen::Vector3f)mVertices[mIndices[counter][2]]);
		}

		++counter;
	}
	return a;
}

float TriMesh::surfaceArea(const Eigen::Affine3f& transform) const
{
	float a = 0;
	for (const Vector3u64& ind : mIndices) {
		a += Triangle::surfaceArea(transform * (Eigen::Vector3f)mVertices[ind[0]],
								   transform * (Eigen::Vector3f)mVertices[ind[1]],
								   transform * (Eigen::Vector3f)mVertices[ind[2]]);
	}
	return a;
}

TriMesh::Collision TriMesh::checkCollision(const Ray& ray)
{
	PR_ASSERT(mKDTree, "kdTree has to be valid");
	TriMesh::Collision r;
	r.Successful = mKDTree
					   ->checkCollision(ray, r.Index, r.Point,
										[this](const Ray& ray, FacePoint& point, uint64 f, float& t) {
											return Triangle::intersect(ray, this->getFace(f), point, t); // Major bottleneck!
										});
	return r;
}

float TriMesh::collisionCost() const
{
	return faceCount();
}

TriMesh::FacePointSample TriMesh::sampleFacePoint(const Eigen::Vector3f& rnd) const
{
	uint32 fi = Projection::map<uint32>(rnd(0), 0, mIndices.size() - 1);
	auto bary = Projection::triangle(rnd(1), rnd(2));

	Face face = getFace(fi);

	Eigen::Vector3f vec;
	Eigen::Vector3f n;
	Eigen::Vector2f uv;
	face.interpolate(bary(0), bary(1), vec, n, uv);

	TriMesh::FacePointSample r;
	r.Point.P	  = vec;
	r.Point.Ng	 = n;
	r.Point.UVW	= Eigen::Vector3f(uv(0), uv(1), 0);
	r.MaterialSlot = face.MaterialSlot;
	r.PDF_A		   = 1.0f / (mIndices.size() * Triangle::surfaceArea(face.V[0], face.V[1], face.V[2]));
	return r;
}
} // namespace PR
