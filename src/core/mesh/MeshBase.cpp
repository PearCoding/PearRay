#include "MeshBase.h"
#include "Profiler.h"
#include "serialization/FileSerializer.h"

namespace PR {
MeshBase::MeshBase()
	: mInfo()
{
}

MeshBase::~MeshBase()
{
}

// Faster variants for triangle, quads only structures?
void MeshBase::buildNormals()
{
	PR_PROFILE_THIS;

	mNormals.resize(mVertices.size());
	std::fill(mNormals.begin(), mNormals.end(), 0.0f);

	for (size_t face = 0; face < faceCount(); ++face) {
		size_t indInd = faceIndexOffset(face);
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

void MeshBase::flipNormals()
{
	PR_PROFILE_THIS;

	for (size_t i = 0; i < mNormals.size(); ++i)
		mNormals[i] *= -1;
}

void MeshBase::setFaceVertexCount(const std::vector<uint8>& faceVertexCount)
{
	PR_PROFILE_THIS;

	mFaceIndexOffset.resize(faceVertexCount.size());
	mInfo.TriangleCount = 0;
	mInfo.QuadCount		= 0;

	uint32 counter = 0;
	for (size_t i = 0; i < faceVertexCount.size(); ++i) {
		mFaceIndexOffset[i] = counter;
		uint32 faceElems	= static_cast<uint32>(faceVertexCount.at(i));

		if (faceElems == 3)
			++mInfo.TriangleCount;
		else if (faceElems == 4)
			++mInfo.QuadCount;
		else
			PR_ASSERT(false, "Only triangles and quads supported");

		counter += faceElems;
	}
}

std::vector<uint32> MeshBase::faceVertexCounts() const
{
	std::vector<uint32> array;
	array.resize(faceCount());
	for (size_t i = 0; i < array.size(); ++i)
		array[i] = static_cast<uint32>(faceVertexCount(i));

	return array;
}

float MeshBase::faceArea(size_t f, const Eigen::Affine3f& tm) const
{
	PR_PROFILE_THIS;

	size_t faceElems = faceVertexCount(f);

	uint32 off		  = faceIndexOffset(f);
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

float MeshBase::surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const
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

float MeshBase::surfaceArea(const Eigen::Affine3f& transform) const
{
	PR_PROFILE_THIS;

	float a = 0;
	for (size_t counter = 0; counter < faceCount(); ++counter)
		a += faceArea(counter, transform);
	return a;
}

BoundingBox MeshBase::constructBoundingBox() const
{
	BoundingBox box;
	for (size_t i = 0; i < nodeCount(); ++i)
		box.combine(vertex(i));

	return box;
}

// TODO: This breaks user attributes
void MeshBase::triangulate()
{
	PR_PROFILE_THIS;

	if (quadCount() == 0)
		return;

	size_t new_facecount = triangleCount() + quadCount() * 2;

	std::vector<uint32> new_indices(new_facecount * 3);
	std::vector<uint32> new_mats;
	if (mInfo.Features & MF_HAS_MATERIAL)
		new_mats.resize(new_facecount);

	size_t indC = 0;
	size_t facC = 0;

	for (size_t face = 0; face < faceCount(); ++face) {
		bool isQuad	  = faceVertexCount(face) == 4;
		size_t indInd = faceIndexOffset(face);

		// Triangle
		new_indices[indC]	  = mIndices[indInd];
		new_indices[indC + 1] = mIndices[indInd + 1];
		new_indices[indC + 2] = mIndices[indInd + 2];
		if (mInfo.Features & MF_HAS_MATERIAL)
			new_mats[facC] = mMaterialSlots[face];
		indC += 3;
		++facC;

		if (isQuad) {
			// Second triangle
			new_indices[indC]	  = mIndices[indInd];
			new_indices[indC + 1] = mIndices[indInd + 2];
			new_indices[indC + 2] = mIndices[indInd + 3];

			if (mInfo.Features & MF_HAS_MATERIAL)
				new_mats[facC] = mMaterialSlots[face];
			indC += 3;
			++facC;
		}
	}

	PR_ASSERT(new_facecount * 3 == indC, "Wrong triangulation!");
	PR_ASSERT(new_facecount == facC, "Wrong triangulation!");

	mInfo.TriangleCount = new_facecount;
	mInfo.QuadCount		= 0;
	mIndices			= new_indices;
	mFaceIndexOffset	= std::vector<uint32>();
	mMaterialSlots		= new_mats;
}

void MeshBase::serialize(Serializer& serializer)
{
	serializer | mInfo.Features
		| mInfo.NodeCount
		| mInfo.TriangleCount
		| mInfo.QuadCount
		| mVertices
		| mNormals
		| mUVs
		| mVelocities
		| mMaterialSlots
		| mIndices
		| mFaceIndexOffset
		| mUserVertexID
		| mUserFaceID
		| mUserVertexAttribs
		| mUserFaceAttribs
		| mUserVertexAttribsU32
		| mUserFaceAttribsU32
		| mUserVertexAttribsU8
		| mUserFaceAttribsU8;
}

size_t MeshBase::addUserVertexAttrib(const std::string& name, const std::vector<float>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * nodeCount(), "Expected vertex attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserVertexAttribs.size();
	mUserVertexAttribs.push_back(cnt);
	mUserVertexID[name] = id;
	return id;
}

size_t MeshBase::addUserFaceAttrib(const std::string& name, const std::vector<float>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * faceCount(), "Expected face attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserFaceAttribs.size();
	mUserFaceAttribs.push_back(cnt);
	mUserFaceID[name] = id;
	return id;
}

size_t MeshBase::addUserVertexAttribU32(const std::string& name, const std::vector<uint32>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * nodeCount(), "Expected vertex attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserVertexAttribsU32.size();
	mUserVertexAttribsU32.push_back(cnt);
	mUserVertexID[name] = id;
	return id;
}

size_t MeshBase::addUserFaceAttribU32(const std::string& name, const std::vector<uint32>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * faceCount(), "Expected face attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserFaceAttribsU32.size();
	mUserFaceAttribsU32.push_back(cnt);
	mUserFaceID[name] = id;
	return id;
}

size_t MeshBase::addUserVertexAttribU8(const std::string& name, const std::vector<uint8>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * nodeCount(), "Expected vertex attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserVertexAttribsU8.size();
	mUserVertexAttribsU8.push_back(cnt);
	mUserVertexID[name] = id;
	return id;
}

size_t MeshBase::addUserFaceAttribU8(const std::string& name, const std::vector<uint8>& cnt, size_t channels)
{
	PR_ASSERT(cnt.size() == channels * faceCount(), "Expected face attribute to have the same count of vertices");
	PR_UNUSED(channels);

	size_t id = mUserFaceAttribsU8.size();
	mUserFaceAttribsU8.push_back(cnt);
	mUserFaceID[name] = id;
	return id;
}

size_t MeshBase::userVertexID(const std::string& name, bool& found) const
{
	found = mUserVertexID.count(name) > 0;
	return found ? mUserVertexID.at(name) : 0;
}

size_t MeshBase::userFaceID(const std::string& name, bool& found) const
{
	found = mUserFaceID.count(name) > 0;
	return found ? mUserFaceID.at(name) : 0;
}

template <typename T>
static inline size_t vectorSize(const std::vector<T>& ptr)
{
	return sizeof(T) * ptr.size();
}
size_t MeshBase::memoryFootprint() const
{
	size_t size = vectorSize(mVertices);
	size += vectorSize(mNormals);
	size += vectorSize(mUVs);
	size += vectorSize(mVelocities);
	size += vectorSize(mIndices);
	size += vectorSize(mFaceIndexOffset);
	for (const auto& v : mUserVertexAttribs)
		size += vectorSize(v);
	for (const auto& v : mUserFaceAttribs)
		size += vectorSize(v);
	for (const auto& v : mUserVertexAttribsU32)
		size += vectorSize(v);
	for (const auto& v : mUserFaceAttribsU32)
		size += vectorSize(v);
	for (const auto& v : mUserVertexAttribsU8)
		size += vectorSize(v);
	for (const auto& v : mUserFaceAttribsU8)
		size += vectorSize(v);
	return size;
}
} // namespace PR
