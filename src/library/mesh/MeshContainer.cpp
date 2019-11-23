#include "MeshContainer.h"
#include "Profiler.h"

namespace PR {
MeshContainer::MeshContainer()
	: mFeatures(0)
	, mTriangleCount(0)
	, mQuadCount(0)
{
}

MeshContainer::~MeshContainer()
{
}

void MeshContainer::setFaceVertexCount(const std::vector<uint8>& faceVertexCount)
{
	PR_PROFILE_THIS;

	mFaceIndexOffset.resize(faceVertexCount.size());
	mTriangleCount = 0;
	mQuadCount	 = 0;

	uint32 counter = 0;
	for (size_t i = 0; i < faceVertexCount.size(); ++i) {
		mFaceIndexOffset[i] = counter;
		uint32 faceElems	= static_cast<uint32>(faceVertexCount.at(i));

		if (faceElems == 3)
			++mTriangleCount;
		else if (faceElems == 4)
			++mQuadCount;
		else
			PR_ASSERT(false, "Only triangles and quads supported");

		counter += faceElems;
	}
}

float MeshContainer::faceArea(size_t f, const Eigen::Affine3f& tm) const
{
	PR_PROFILE_THIS;

	size_t faceElems = faceVertexCount(f);

	uint32 off		  = mFaceIndexOffset[f];
	const uint32 ind1 = mIndices[off];
	const uint32 ind2 = mIndices[off + 1];
	const uint32 ind3 = mIndices[off + 2];

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

	if (faceElems == 3)
		return Triangle::surfaceArea(p1, p2, p3);
	else {
		const uint32 ind4 = mIndices[off + 3];
		const Vector3f p4 = Transform::apply(tm.matrix(),
											 Vector3f(mVertices[0][ind4],
													  mVertices[1][ind4],
													  mVertices[2][ind4]));
		return Quad::surfaceArea(p1, p2, p3, p4);
	}
}

float MeshContainer::surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const
{
	PR_PROFILE_THIS;

	if (!(features() & MF_HAS_MATERIAL)) {
		if (slot == 0)
			return surfaceArea(transform);
		else
			return 0;
	}

	float a		   = 0;
	size_t counter = 0;
	for (uint32 mat : mMaterialSlots) {
		if (mat == slot) {
			a += faceArea(counter, transform);
		}

		++counter;
	}
	return a;
}

float MeshContainer::surfaceArea(const Eigen::Affine3f& transform) const
{
	PR_PROFILE_THIS;

	float a = 0;
	for (size_t counter = 0; counter < faceCount(); ++counter) {
		a += faceArea(counter, transform);
	}
	return a;
}
BoundingBox MeshContainer::constructBoundingBox() const
{
	BoundingBox box;
	for (size_t i = 0; i < nodeCount(); ++i)
		box.combine(vertex(i));

	return box;
}

void MeshContainer::triangulate()
{
	// FIXME: TODO!!!
	PR_ASSERT(quadCount() == 0, "Triangulation is not yet implemented!");
}
} // namespace PR
