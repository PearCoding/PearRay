namespace PR {

inline Face TriMesh::getFace(size_t i) const
{
	PR_ASSERT(i < faceCount(), "Invalid face access!");

	const Vector3u64& ind = mIndices[i];

	Face f;
	for (int j = 0; j < 3; ++j) {
		f.V[j] = mVertices[ind[j]];
		f.N[j] = mNormals[ind[j]];

		if (features() & TMF_HAS_UV) {
			f.UV[j] = mUVs[ind[j]];
		} else {
			f.UV[j] = Eigen::Vector2f(0.0f, 0.0f);
		}
	}

	f.MaterialSlot = getFaceMaterial(i);

	return f;
}

inline uint32 TriMesh::getFaceMaterial(uint64 index) const
{
	if (features() & TMF_HAS_MATERIAL) {
		return mMaterials[index];
	} else {
		return 0;
	}
}

inline bool TriMesh::isValid() const
{
	return mVertices.size() >= 3 && !mIndices.empty() && mVertices.size() == mNormals.size();
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
