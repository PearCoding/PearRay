#include "TriMesh.h"
#include "Face.h"
#include "Triangle.h"
#include "container/kdTree.h"
#include "material/Material.h"
#include "math/Projection.h"

#include <boost/range/irange.hpp>

namespace PR {
typedef PR::kdTree<uint64, TriMesh, false> TriKDTree;
TriMesh::TriMesh()
	: mKDTree(nullptr)
{
}

TriMesh::~TriMesh()
{
	clear();
}

Face TriMesh::getFace(size_t i) const
{
	PR_ASSERT(i < faceCount(), "Invalid face access!");

	const Vector3u64& ind = mIndices[i];

	Face f;
	for (int j = 0; j < 3; ++j) {
		const Vector3f& v = mVertices[ind[j]];
		const Vector3f& n = mNormals[ind[j]];

		f.V[j] = v;
		f.N[j] = n;

		if (features() & TMF_HAS_UV) {
			const Vector2f& uv = mUVs[ind[j]];
			f.UV[j]			   = uv;
		} else {
			f.UV[j] = Eigen::Vector2f(0.0f, 0.0f);
		}
	}

	f.MaterialSlot = getFaceMaterial(i);

	return f;
}

uint32 TriMesh::getFaceMaterial(uint64 index) const
{
	if (features() & TMF_HAS_MATERIAL) {
		return mMaterials[index];
	} else {
		return 0;
	}
}

bool TriMesh::isValid() const
{
	return mVertices.size() >= 3 && !mIndices.empty() && mVertices.size() == mNormals.size();
}

void TriMesh::clear()
{
	if (mKDTree) {
		delete reinterpret_cast<TriKDTree*>(mKDTree);
		mKDTree = nullptr;
	}
}

constexpr float TriangleTestCost = 100.0f;
void TriMesh::build()
{
	PR_ASSERT(isValid(), "Mesh has to be valid before build()!");

	// Build internal KDtree
	if (mKDTree) {
		delete reinterpret_cast<TriKDTree*>(mKDTree);
		mKDTree = nullptr;
	}

	mKDTree = new TriKDTree(this,
							[](TriMesh* mesh, uint64 f) {
								return Triangle::getBoundingBox(mesh->mVertices[mesh->mIndices[f][0]],
																mesh->mVertices[mesh->mIndices[f][1]],
																mesh->mVertices[mesh->mIndices[f][2]]);
							},
							[](TriMesh*, uint64 f) {
								return TriangleTestCost;
							});

	reinterpret_cast<TriKDTree*>(mKDTree)->enableCostForLeafDetection(false);
	reinterpret_cast<TriKDTree*>(mKDTree)->setMinimumObjectsForLeaf(8);

	auto it = boost::irange<uint64>(0, mIndices.size());
	reinterpret_cast<TriKDTree*>(mKDTree)->build(it.begin(), it.end(), mIndices.size());

	PR_LOG(L_INFO) << "Mesh KDtree [depth="
				   << reinterpret_cast<TriKDTree*>(mKDTree)->depth()
				   << ", elements=" << faceCount()
				   << ", leafs=" << reinterpret_cast<TriKDTree*>(mKDTree)->leafCount()
				   << ", elementsPerLeaf=[avg:" << reinterpret_cast<TriKDTree*>(mKDTree)->avgElementsPerLeaf()
				   << ", min:" << reinterpret_cast<TriKDTree*>(mKDTree)->minElementsPerLeaf()
				   << ", max:" << reinterpret_cast<TriKDTree*>(mKDTree)->maxElementsPerLeaf()
				   << "]]" << std::endl;

	if (!reinterpret_cast<TriKDTree*>(mKDTree)->isEmpty())
		mBoundingBox = reinterpret_cast<TriKDTree*>(mKDTree)->boundingBox();

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
			a += Triangle::surfaceArea(transform * (Eigen::Vector3f)mVertices[mIndices[counter][0]],
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
	r.Successful = reinterpret_cast<TriKDTree*>(mKDTree)->checkCollision(ray, r.Index, r.Point,
																		 [](TriMesh* mesh, const Ray& ray, FacePoint& point, uint64 f) {
																			 float t;
																			 return Triangle::intersect(ray, mesh->getFace(f), point, t); // Major bottleneck!
																		 });
	return r;
}

float TriMesh::collisionCost() const
{
	return reinterpret_cast<TriKDTree*>(mKDTree)->depth() + TriangleTestCost * reinterpret_cast<TriKDTree*>(mKDTree)->avgElementsPerLeaf();
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
