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

inline void MeshContainer::setMaterials(const std::vector<uint32>& f)
{
	mMaterials = f;
	if (!f.empty())
		mFeatures |= MF_HAS_MATERIAL;
}

inline size_t MeshContainer::faceVertexCount(size_t face) const
{
	return (face + 1) >= mFaceOffset.size()
			   ? static_cast<size_t>(mIndices.size() - static_cast<int32>(mFaceOffset[face]))
			   : static_cast<size_t>(mFaceOffset[face + 1] - static_cast<int32>(mFaceOffset[face]));
}

inline Face MeshContainer::getFace(uint32 index) const
{
	size_t faceElems = faceVertexCount(index);
	PR_ASSERT(faceElems == 3 || faceElems == 4, "Only triangles and quads are supported.");

	uint32 indInd = mFaceOffset[index];

	Face f;
	f.IsQuad			= (faceElems == 4);
	const uint32 ind[4] = { mIndices[indInd],
							mIndices[indInd + 1],
							mIndices[indInd + 2],
							f.IsQuad ? mIndices[indInd + 3] : 0 };

	for (size_t j = 0; j < faceElems; ++j) {
		f.V[j](0) = mVertices[0][ind[j]];
		f.V[j](1) = mVertices[1][ind[j]];
		f.V[j](2) = mVertices[2][ind[j]];

		f.N[j](0) = mNormals[0][ind[j]];
		f.N[j](1) = mNormals[1][ind[j]];
		f.N[j](2) = mNormals[2][ind[j]];

		if (features() & MF_HAS_UV) {
			f.UV[j](0) = mUVs[0][ind[j]];
			f.UV[j](1) = mUVs[1][ind[j]];
		} else {
			f.UV[j] = Vector2f(0, 0);
		}
	}

	f.MaterialSlot = (features() & MF_HAS_MATERIAL) ? mMaterials.at(index) : 0;

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
