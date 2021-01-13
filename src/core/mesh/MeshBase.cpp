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

void MeshBase::buildSmoothNormals()
{
	const auto& indices = vertexComponentIndices(MeshComponent::Vertex);

	std::vector<float> normals;
	normals.resize(nodeCount() * 3);
	std::fill(normals.begin(), normals.end(), 0.0f);

	for (size_t face = 0; face < faceCount(); ++face) {
		size_t indInd = faceIndexOffset(face);
		size_t elems  = faceVertexCount(face);

		Vector3f n;
		if (elems == 4) {
			Vector3f p0 = vertexComponent3D(MeshComponent::Vertex, indices[indInd]);
			Vector3f p1 = vertexComponent3D(MeshComponent::Vertex, indices[indInd + 1]);
			Vector3f p2 = vertexComponent3D(MeshComponent::Vertex, indices[indInd + 2]);
			Vector3f p3 = vertexComponent3D(MeshComponent::Vertex, indices[indInd + 3]);

			n = Quad::normal(p0, p1, p2, p3);
		} else {
			Vector3f p0 = vertexComponent3D(MeshComponent::Vertex, indices[indInd]);
			Vector3f p1 = vertexComponent3D(MeshComponent::Vertex, indices[indInd + 1]);
			Vector3f p2 = vertexComponent3D(MeshComponent::Vertex, indices[indInd + 2]);

			n = Triangle::normal(p0, p1, p2);
		}

		for (size_t k = 0; k < elems; ++k) {
			uint32 index		   = indices[indInd + k];
			normals[3 * index]	   = n(0);
			normals[3 * index + 1] = n(1);
			normals[3 * index + 2] = n(2);
		}
	}

	// Normalize
	for (size_t i = 0; i < normals.size() / 3; ++i) {
		Vector3f n = Vector3f(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]);
		n.normalize();
		normals[3 * i]	   = n(0);
		normals[3 * i + 1] = n(1);
		normals[3 * i + 2] = n(2);
	}

	setVertexComponent(MeshComponent::Normal, std::move(normals));
}

void MeshBase::flipNormals()
{
	PR_PROFILE_THIS;
	if (!hasVertexComponent(MeshComponent::Normal))
		return;

	auto& c = mVertexComponents[(size_t)MeshComponent::Normal];
	for (size_t i = 0; i < c.Entries.size(); ++i)
		c.Entries[i] *= -1;
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

	const size_t faceElems = faceVertexCount(f);
	const auto& c		   = mVertexComponents[(size_t)MeshComponent::Vertex];

	const uint32 off  = faceIndexOffset(f);
	const uint32 ind1 = c.Indices[off];
	const uint32 ind2 = c.Indices[off + 1];
	const uint32 ind3 = c.Indices[off + 2];

	const Vector3f p1 = Transform::apply(tm.matrix(), vertexComponent3D(MeshComponent::Vertex, ind1));
	const Vector3f p2 = Transform::apply(tm.matrix(), vertexComponent3D(MeshComponent::Vertex, ind2));
	const Vector3f p3 = Transform::apply(tm.matrix(), vertexComponent3D(MeshComponent::Vertex, ind3));

	if (faceElems == 3)
		return Triangle::surfaceArea(p1, p2, p3);
	else {
		const uint32 ind4 = c.Indices[off + 3];
		const Vector3f p4 = Transform::apply(tm.matrix(), vertexComponent3D(MeshComponent::Vertex, ind4));
		return Quad::surfaceArea(p1, p2, p3, p4);
	}
}

float MeshBase::surfaceArea(uint32 matID, const Eigen::Affine3f& transform) const
{
	PR_PROFILE_THIS;

	if (matID == PR_INVALID_ID || !(features() & MeshFeature::Material))
		return surfaceArea(transform);

	float a		   = 0;
	size_t counter = 0;
	for (uint32 mat : mMaterialSlots) {
		if (mat == matID)
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
		box.combine(vertexComponent3D(MeshComponent::Vertex, i));

	return box;
}

void MeshBase::serialize(Serializer& serializer)
{
	serializer | mInfo.Features.value
		| mInfo.NodeCount
		| mInfo.TriangleCount
		| mInfo.QuadCount;

	for (size_t i = 0; i < (size_t)MeshComponent::_COUNT; ++i) {
		serializer | mVertexComponents[i].Entries
			| mVertexComponents[i].Indices;
	}

	serializer | mMaterialSlots
		| mFaceIndexOffset;
}

template <typename T>
static inline size_t vectorSize(const std::vector<T>& ptr)
{
	return sizeof(T) * ptr.size();
}
size_t MeshBase::memoryFootprint() const
{
	size_t size = 0;
	for (size_t i = 0; i < (size_t)MeshComponent::_COUNT; ++i) {
		size += vectorSize(mVertexComponents[i].Entries);
		size += vectorSize(mVertexComponents[i].Indices);
	}
	size += vectorSize(mMaterialSlots);
	size += vectorSize(mFaceIndexOffset);
	return size;
}
} // namespace PR
