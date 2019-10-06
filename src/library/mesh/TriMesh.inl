namespace PR {

inline void TriMesh::setVertices(const std::vector<float>& vx,
								 const std::vector<float>& vy, const std::vector<float>& vz)
{
	mVertices[0] = vx;
	mVertices[1] = vy;
	mVertices[2] = vz;
}

inline void TriMesh::setNormals(const std::vector<float>& vx,
								const std::vector<float>& vy, const std::vector<float>& vz)
{
	mNormals[0] = vx;
	mNormals[1] = vy;
	mNormals[2] = vz;
}

inline void TriMesh::setTangents(const std::vector<float>& vx,
								 const std::vector<float>& vy, const std::vector<float>& vz)
{
	mTangents[0] = vx;
	mTangents[1] = vy;
	mTangents[2] = vz;
}

inline void TriMesh::setBitangents(const std::vector<float>& vx,
								   const std::vector<float>& vy, const std::vector<float>& vz)
{
	mBitangents[0] = vx;
	mBitangents[1] = vy;
	mBitangents[2] = vz;
}

inline void TriMesh::setUVs(const std::vector<float>& u,
							const std::vector<float>& v)
{
	mUVs[0] = u;
	mUVs[1] = v;
}

inline void TriMesh::setVelocities(const std::vector<float>& vx,
								   const std::vector<float>& vy, const std::vector<float>& vz)
{
	mVelocities[0] = vx;
	mVelocities[1] = vy;
	mVelocities[2] = vz;
}

inline void TriMesh::setIndices(const std::vector<uint32>& i1,
								const std::vector<uint32>& i2, const std::vector<uint32>& i3)
{
	mIndices[0] = i1;
	mIndices[1] = i2;
	mIndices[2] = i3;
}

inline Face TriMesh::getFace(uint32 index) const
{
	Face f;
	uint32 ind[3] = { mIndices[0][index],
					  mIndices[1][index],
					  mIndices[2][index] };

	for (int j = 0; j < 3; ++j) {
		f.V[j](0) = mVertices[0][ind[j]];
		f.V[j](1) = mVertices[1][ind[j]];
		f.V[j](2) = mVertices[2][ind[j]];

		f.N[j](0) = mNormals[0][ind[j]];
		f.N[j](1) = mNormals[1][ind[j]];
		f.N[j](2) = mNormals[2][ind[j]];

		if (features() & TMF_HAS_UV) {
			f.UV[j](0) = mUVs[0][ind[j]];
			f.UV[j](1) = mUVs[1][ind[j]];
		} else {
			f.UV[j] = Vector2f(0, 0);
		}
	}

	f.MaterialSlot = getFaceMaterial(index);
	return f;
}

inline uint32 TriMesh::getFaceMaterial(uint32 index) const
{
	if (features() & TMF_HAS_MATERIAL) {
		return mMaterials.at(index);
	} else {
		return 0;
	}
}

inline bool TriMesh::isValid() const
{
	return mVertices[0].size() >= 3
		   && mVertices[0].size() == mVertices[1].size()
		   && mVertices[1].size() == mVertices[2].size()
		   && !mIndices[0].empty()
		   && mIndices[0].size() == mIndices[1].size()
		   && mIndices[1].size() == mIndices[2].size()
		   && mVertices[0].size() == mNormals[0].size()
		   && mNormals[0].size() == mNormals[1].size()
		   && mNormals[1].size() == mNormals[2].size();
}

inline void TriMesh::setIntersectionTestCost(float f)
{
	mIntersectionTestCost = f;
}

inline float TriMesh::intersectionTestCost() const
{
	return mIntersectionTestCost;
}
} // namespace PR
