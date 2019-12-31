// IWYU pragma: private, include "mesh/MeshBase.h"
namespace PR {

inline void MeshBase::setVertices(const std::vector<float>& verts)
{
	mVertices		= verts;
	mInfo.NodeCount = verts.size() / 3;
}

inline void MeshBase::setNormals(const std::vector<float>& norms)
{
	mNormals = norms;
}

inline void MeshBase::setUVs(const std::vector<float>& uvs)
{
	mUVs = uvs;
	if (!uvs.empty())
		mInfo.Features |= MF_HAS_UV;
}

inline void MeshBase::setVelocities(const std::vector<float>& velocities)
{
	mVelocities = velocities;
	if (!velocities.empty())
		mInfo.Features |= MF_HAS_VELOCITY;
}

inline void MeshBase::setIndices(const std::vector<uint32>& indices)
{
	mIndices = indices;
}

inline void MeshBase::setMaterialSlots(const std::vector<uint32>& f)
{
	mMaterialSlots = f;
	if (!f.empty())
		mInfo.Features |= MF_HAS_MATERIAL;
}

inline size_t MeshBase::faceVertexCount(size_t face) const
{
	return (face + 1) >= mFaceIndexOffset.size()
			   ? static_cast<size_t>(mIndices.size() - static_cast<int32>(mFaceIndexOffset[face]))
			   : static_cast<size_t>(mFaceIndexOffset[face + 1] - static_cast<int32>(mFaceIndexOffset[face]));
}

inline Face MeshBase::getFace(uint32 index) const
{
	size_t faceElems = faceVertexCount(index);
	PR_ASSERT(faceElems == 3 || faceElems == 4, "Only triangles and quads are supported.");

	uint32 indInd = mFaceIndexOffset[index];

	Face f;
	f.IsQuad			= (faceElems == 4);
	const uint32 ind[4] = { mIndices.at(indInd),
							mIndices.at(indInd + 1),
							mIndices.at(indInd + 2),
							f.IsQuad ? mIndices.at(indInd + 3) : 0 };

	for (size_t j = 0; j < faceElems; ++j) {
		f.V[j]  = vertex(ind[j]);
		f.N[j]  = normal(ind[j]);
		f.UV[j] = uv(ind[j]);
	}

	f.MaterialSlot = materialSlot(index);

	return f;
}

inline bool MeshBase::isValid() const
{
	return faceCount() > 0
		   && (triangleCount() * 3 + quadCount() * 4) == mIndices.size()
		   && nodeCount() > 3
		   && (mVertices.size() % 3) == 0
		   && mVertices.size() == mNormals.size();
}

} // namespace PR
