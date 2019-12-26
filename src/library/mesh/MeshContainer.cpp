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

void MeshContainer::buildNormals()
{
	PR_PROFILE_THIS;

	mNormals.resize(mVertices.size());
	std::fill(mNormals.begin(), mNormals.end(), 0.0f);

	for (size_t face = 0; face < faceCount(); ++face) {
		size_t indInd = mFaceIndexOffset[face];
		size_t elems  = faceVertexCount(face);

		Vector3f n;
		if (elems == 4) {
			Vector3f p0 = vertex(mIndices[indInd]);
			Vector3f p1 = vertex(mIndices[indInd + 1]);
			Vector3f p2 = vertex(mIndices[indInd + 2]);
			Vector3f p3 = vertex(mIndices[indInd + 3]);

			n = (p2 - p0).cross(p3 - p1).normalized();
		} else {
			Vector3f p0 = vertex(mIndices[indInd]);
			Vector3f p1 = vertex(mIndices[indInd + 1]);
			Vector3f p2 = vertex(mIndices[indInd + 2]);

			n = (p1 - p0).cross(p2 - p0).normalized();
		}

		for (size_t k = 0; k < elems; ++k) {
			uint32 index			= mIndices[indInd + k];
			mNormals[3 * index]		= n(0);
			mNormals[3 * index + 1] = n(1);
			mNormals[3 * index + 2] = n(2);
		}
	}

	// Normalize
	for (size_t i = 0; i < mNormals.size() / 3; ++i) {
		Vector3f n = Vector3f(mNormals[3 * i], mNormals[3 * i + 1], mNormals[3 * i + 2]);
		n.normalize();
		mNormals[3 * i]		= n(0);
		mNormals[3 * i + 1] = n(1);
		mNormals[3 * i + 2] = n(2);
	}
}

void MeshContainer::flipNormals()
{
	PR_PROFILE_THIS;

	for (size_t i = 0; i < mNormals.size(); ++i)
		mNormals[i] *= -1;
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

std::vector<uint32> MeshContainer::faceVertexCounts() const
{
	std::vector<uint32> array;
	array.reserve(mFaceIndexOffset.size());
	for (size_t i = 0; i < mFaceIndexOffset.size(); ++i)
		array.push_back(static_cast<uint32>(faceVertexCount(i)));

	return array;
}

float MeshContainer::faceArea(size_t f, const Eigen::Affine3f& tm) const
{
	PR_PROFILE_THIS;

	size_t faceElems = faceVertexCount(f);

	uint32 off		  = mFaceIndexOffset[f];
	const uint32 ind1 = mIndices[off];
	const uint32 ind2 = mIndices[off + 1];
	const uint32 ind3 = mIndices[off + 2];

	const Vector3f p1 = Transform::apply(tm.matrix(), vertex(ind1));

	const Vector3f p2 = Transform::apply(tm.matrix(), vertex(ind2));

	const Vector3f p3 = Transform::apply(tm.matrix(), vertex(ind3));

	if (faceElems == 3)
		return Triangle::surfaceArea(p1, p2, p3);
	else {
		const uint32 ind4 = mIndices[off + 3];
		const Vector3f p4 = Transform::apply(tm.matrix(), vertex(ind4));
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
		if (mat == slot)
			a += faceArea(counter, transform);

		++counter;
	}
	return a;
}

float MeshContainer::surfaceArea(const Eigen::Affine3f& transform) const
{
	PR_PROFILE_THIS;

	float a = 0;
	for (size_t counter = 0; counter < faceCount(); ++counter)
		a += faceArea(counter, transform);
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
	PR_PROFILE_THIS;

	if (quadCount() == 0)
		return;

	size_t new_facecount = triangleCount() + quadCount() * 2;

	std::vector<uint32> new_indices(new_facecount * 3);
	std::vector<uint32> new_faceIndexOffset(new_facecount);
	std::vector<uint32> new_mats;
	if (mFeatures & MF_HAS_MATERIAL)
		new_mats.resize(new_faceIndexOffset.size());

	size_t indC = 0;
	size_t facC = 0;

	for (size_t face = 0; face < faceCount(); ++face) {
		bool isQuad   = faceVertexCount(face) == 4;
		size_t indInd = mFaceIndexOffset[face];

		// Triangle
		new_indices[indC]		  = mIndices[indInd];
		new_indices[indC + 1]	 = mIndices[indInd + 1];
		new_indices[indC + 2]	 = mIndices[indInd + 2];
		new_faceIndexOffset[facC] = static_cast<uint32>(indC);
		if (mFeatures & MF_HAS_MATERIAL)
			new_mats[facC] = mMaterialSlots[face];
		indC += 3;
		++facC;

		if (isQuad) {
			// Second triangle
			new_indices[indC]	 = mIndices[indInd];
			new_indices[indC + 1] = mIndices[indInd + 2];
			new_indices[indC + 2] = mIndices[indInd + 3];

			new_faceIndexOffset[facC] = static_cast<uint32>(indC);
			if (mFeatures & MF_HAS_MATERIAL)
				new_mats[facC] = mMaterialSlots[face];
			indC += 3;
			++facC;
		}
	}

	PR_ASSERT(new_facecount * 3 == indC, "Wrong triangulation!");
	PR_ASSERT(new_facecount == facC, "Wrong triangulation!");

	mTriangleCount   = new_facecount;
	mQuadCount		 = 0;
	mIndices		 = new_indices;
	mFaceIndexOffset = new_faceIndexOffset;
	mMaterialSlots   = new_mats;
}
} // namespace PR
