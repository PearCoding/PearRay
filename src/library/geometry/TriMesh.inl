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

inline FacePackage TriMesh::getFaces(const vuint32& indices) const
{
	FacePackage pkg;
	vuint32 ind[3] = { load_from_container(indices, mIndices[0]),
					   load_from_container(indices, mIndices[1]),
					   load_from_container(indices, mIndices[2]) };

	for (int j = 0; j < 3; ++j) {
		pkg.Vx[j] = load_from_container(ind[j], mVertices[0]);
		pkg.Vy[j] = load_from_container(ind[j], mVertices[1]);
		pkg.Vz[j] = load_from_container(ind[j], mVertices[2]);

		pkg.Nx[j] = load_from_container(ind[j], mNormals[0]);
		pkg.Ny[j] = load_from_container(ind[j], mNormals[1]);
		pkg.Nz[j] = load_from_container(ind[j], mNormals[2]);

		if (features() & TMF_HAS_UV) {
			pkg.U[j] = load_from_container(ind[j], mUVs[0]);
			pkg.V[j] = load_from_container(ind[j], mUVs[1]);
		} else {
			pkg.U[j] = simdpp::make_float(0);
			pkg.V[j] = simdpp::make_float(0);
		}
	}

	pkg.MaterialSlot = getFaceMaterials(indices);
	return pkg;
}

inline vuint32 TriMesh::getFaceMaterials(const vuint32& indices) const
{
	if (features() & TMF_HAS_MATERIAL) {
		return load_from_container(indices, mMaterials);
	} else {
		return simdpp::make_uint(0);
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
