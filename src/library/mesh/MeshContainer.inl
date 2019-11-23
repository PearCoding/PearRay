// IWYU pragma: private, include "mesh/MeshContainer.h"
namespace PR {

inline void MeshContainer::setVertices(const std::vector<float>& vx,
									   const std::vector<float>& vy, const std::vector<float>& vz)
{
	mVertices[0] = vx;
	mVertices[1] = vy;
	mVertices[2] = vz;
}

inline void MeshContainer::setNormals(const std::vector<float>& vx,
									  const std::vector<float>& vy, const std::vector<float>& vz)
{
	mNormals[0] = vx;
	mNormals[1] = vy;
	mNormals[2] = vz;
}

inline void MeshContainer::setUVs(const std::vector<float>& u,
								  const std::vector<float>& v)
{
	mUVs[0] = u;
	mUVs[1] = v;
	if (!u.empty() && !v.empty())
		mFeatures |= MF_HAS_UV;
}

inline void MeshContainer::setVelocities(const std::vector<float>& vx,
										 const std::vector<float>& vy, const std::vector<float>& vz)
{
	mVelocities[0] = vx;
	mVelocities[1] = vy;
	mVelocities[2] = vz;
	if (!vx.empty() && !vy.empty() && !vz.empty())
		mFeatures |= MF_HAS_VELOCITY;
}

inline void MeshContainer::setIndices(const std::vector<uint32>& indices)
{
	mIndices = indices;
}

inline void MeshContainer::setMaterialSlots(const std::vector<uint32>& f)
{
	mMaterialSlots = f;
	if (!f.empty())
		mFeatures |= MF_HAS_MATERIAL;
}

inline size_t MeshContainer::faceVertexCount(size_t face) const
{
	return (face + 1) >= mFaceIndexOffset.size()
			   ? static_cast<size_t>(mIndices.size() - static_cast<int32>(mFaceIndexOffset[face]))
			   : static_cast<size_t>(mFaceIndexOffset[face + 1] - static_cast<int32>(mFaceIndexOffset[face]));
}

inline Face MeshContainer::getFace(uint32 index) const
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

inline bool MeshContainer::isValid() const
{
	return faceCount() > 0
		   && (triangleCount() * 3 + quadCount() * 4) == mIndices.size()
		   && mVertices[0].size() >= 3
		   && mVertices[0].size() == mVertices[1].size()
		   && mVertices[1].size() == mVertices[2].size()
		   && mVertices[0].size() == mNormals[0].size()
		   && mNormals[0].size() == mNormals[1].size()
		   && mNormals[1].size() == mNormals[2].size();
}

} // namespace PR
