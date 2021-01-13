// IWYU pragma: private, include "mesh/MeshBase.h"
namespace PR {

inline void MeshBase::setVertexComponent(MeshComponent component, const std::vector<float>& entries)
{
	mVertexComponents[(size_t)component].Entries = entries;
	handleInfo(component);
}

inline void MeshBase::setVertexComponent(MeshComponent component, std::vector<float>&& entries)
{
	mVertexComponents[(size_t)component].Entries = std::move(entries);
	handleInfo(component);
}

inline void MeshBase::setVertexComponentIndices(MeshComponent component, const std::vector<uint32>& indices)
{
	mVertexComponents[(size_t)component].Indices = indices;
}

inline void MeshBase::setVertexComponentIndices(MeshComponent component, std::vector<uint32>&& indices)
{
	mVertexComponents[(size_t)component].Indices = std::move(indices);
}

inline Vector3f MeshBase::vertexComponent3D(MeshComponent component, size_t ind) const
{
	PR_ASSERT(isComponent3D(component), "Expected a 3d component");
	const auto& c = mVertexComponents[(size_t)component];
	return Vector3f(c.Entries[3 * ind + 0], c.Entries[3 * ind + 1], c.Entries[3 * ind + 2]);
}

inline Vector2f MeshBase::vertexComponent2D(MeshComponent component, size_t ind) const
{
	PR_ASSERT(isComponent2D(component), "Expected a 2d component");
	const auto& c = mVertexComponents[(size_t)component];
	return Vector2f(c.Entries[2 * ind + 0], c.Entries[2 * ind + 1]);
}

inline float MeshBase::vertexComponent1D(MeshComponent component, size_t ind) const
{
	PR_ASSERT(isComponent1D(component), "Expected a 1d component");
	return mVertexComponents[(size_t)component].Entries[ind];
}

inline void MeshBase::setMaterialSlots(const std::vector<uint32>& f)
{
	mMaterialSlots = f;
	if (!f.empty())
		mInfo.Features |= MeshFeature::Material;
}

inline void MeshBase::setMaterialSlots(std::vector<uint32>&& f)
{
	mMaterialSlots = std::move(f);
	if (!mMaterialSlots.empty())
		mInfo.Features |= MeshFeature::Material;
}

inline void MeshBase::assumeTriangular(size_t faceCount)
{
	mInfo.QuadCount		= 0;
	mInfo.TriangleCount = faceCount;
	mFaceIndexOffset	= std::vector<uint32>();
}

inline void MeshBase::assumeQuadrangular(size_t faceCount)
{
	mInfo.QuadCount		= faceCount;
	mInfo.TriangleCount = 0;
	mFaceIndexOffset	= std::vector<uint32>();
}

inline void MeshBase::setFaceIndexOffsets(const std::vector<uint32>& faceIndexOffsets)
{
	mFaceIndexOffset = faceIndexOffsets;
}

inline void MeshBase::setFaceIndexOffsets(std::vector<uint32>&& faceIndexOffsets)
{
	mFaceIndexOffset = std::move(faceIndexOffsets);
}

inline size_t MeshBase::faceVertexCount(size_t face) const
{
	if (mInfo.QuadCount == 0)
		return 3;
	else if (mInfo.TriangleCount == 0)
		return 4;
	else
		return (face + 1) >= mFaceIndexOffset.size()
				   ? static_cast<size_t>(vertexComponentIndices(MeshComponent::Vertex).size() - static_cast<int32>(mFaceIndexOffset[face]))
				   : static_cast<size_t>(mFaceIndexOffset[face + 1] - static_cast<int32>(mFaceIndexOffset[face]));
}

inline uint32 MeshBase::faceIndexOffset(uint32 f) const
{
	if (mInfo.QuadCount == 0)
		return f * 3;
	else if (mInfo.TriangleCount == 0)
		return f * 4;
	else
		return mFaceIndexOffset[f];
}

inline Face MeshBase::getFace(uint32 index) const
{
	using IndexArray = Eigen::Array<uint32, 4, 1>;

	size_t faceElems = faceVertexCount(index);
	PR_ASSERT(faceElems == 3 || faceElems == 4, "Only triangles and quads are supported.");

	uint32 indInd	 = faceIndexOffset(index);
	const auto& inds = vertexComponentIndices(MeshComponent::Vertex);

	Face f;
	f.IsQuad			  = (faceElems == 4);
	const IndexArray vInd = IndexArray(inds[indInd],
									   inds[indInd + 1],
									   inds[indInd + 2],
									   f.IsQuad ? inds[indInd + 3] : 0);

	for (size_t j = 0; j < faceElems; ++j)
		f.V[j] = vertexComponent3D(MeshComponent::Vertex, vInd[j]);

	if (mInfo.Features & MeshFeature::Normal) {
		const auto& nInds	  = vertexComponentIndices(MeshComponent::Normal);
		const IndexArray nInd = nInds.empty() ? vInd : IndexArray(nInds[indInd], nInds[indInd + 1], nInds[indInd + 2], f.IsQuad ? nInds[indInd + 3] : 0);

		for (size_t j = 0; j < faceElems; ++j)
			f.N[j] = vertexComponent3D(MeshComponent::Normal, nInd[j]);
	}

	if (mInfo.Features & MeshFeature::Texture) {
		const auto& tInds	  = vertexComponentIndices(MeshComponent::Texture);
		const IndexArray tInd = tInds.empty() ? vInd : IndexArray(tInds[indInd], tInds[indInd + 1], tInds[indInd + 2], f.IsQuad ? tInds[indInd + 3] : 0);
		for (size_t j = 0; j < faceElems; ++j)
			f.UV[j] = vertexComponent2D(MeshComponent::Texture, tInd[j]);
	}

	f.MaterialSlot = materialSlot(index);

	return f;
}

inline bool MeshBase::isValid() const
{
	const size_t expectedSize = triangleCount() * 3 + quadCount() * 4;

	// The vertex index buffer has to be always available
	if (mVertexComponents[(size_t)MeshComponent::Vertex].Indices.size() != expectedSize)
		return false;

	for (size_t c = 0; c < (size_t)MeshComponent::_COUNT; ++c) {
		if (mVertexComponents[c].Indices.empty())
			continue;

		if (mVertexComponents[c].Indices.size() != expectedSize)
			return false;

		// Can not be empty!
		if (mVertexComponents[c].Entries.size() == 0)
			return false;

		// Has to be a multiple of two
		if (isComponent2D((MeshComponent)c) && mVertexComponents[c].Entries.size() % 2 != 0)
			return false;

		// Has to be a multiple of three
		if (isComponent3D((MeshComponent)c) && mVertexComponents[c].Entries.size() % 3 != 0)
			return false;
	}

	PR_ASSERT(!(mInfo.Features & MeshFeature::Normal) == mVertexComponents[(size_t)MeshComponent::Normal].Entries.empty(), "Expected normals to be present if feature is present!");
	PR_ASSERT(!(mInfo.Features & MeshFeature::Texture) == mVertexComponents[(size_t)MeshComponent::Texture].Entries.empty(), "Expected texture coordinates to be present if feature is present!");
	PR_ASSERT(!(mInfo.Features & MeshFeature::Weight) == mVertexComponents[(size_t)MeshComponent::Weight].Entries.empty(), "Expected weights to be present if feature is present!");
	PR_ASSERT(!(mInfo.Features & MeshFeature::Velocity) == mVertexComponents[(size_t)MeshComponent::Velocity].Entries.empty(), "Expected velocities to be present if feature is present!");
	PR_ASSERT(!(mInfo.Features & MeshFeature::Material) == mMaterialSlots.empty(), "Expected normals to be present if feature is present!");

	return faceCount() > 0 && nodeCount() > 3;
}

void MeshBase::handleInfo(MeshComponent component)
{
	switch (component) {
	default:
	case MeshComponent::Vertex:
		mInfo.NodeCount = mVertexComponents[(size_t)component].Entries.size() / 3;
		break;
	case MeshComponent::Normal:
		if (!mVertexComponents[(size_t)component].Entries.empty())
			mInfo.Features |= MeshFeature::Normal;
		break;
	case MeshComponent::Texture:
		if (!mVertexComponents[(size_t)component].Entries.empty())
			mInfo.Features |= MeshFeature::Texture;
		break;
	case MeshComponent::Weight:
		if (!mVertexComponents[(size_t)component].Entries.empty())
			mInfo.Features |= MeshFeature::Weight;
		break;
	case MeshComponent::Velocity:
		if (!mVertexComponents[(size_t)component].Entries.empty())
			mInfo.Features |= MeshFeature::Velocity;
		break;
	}
}
} // namespace PR
