#pragma once

#include "geometry/Face.h"

#include <vector>

namespace PR {
enum MeshFeatures : uint32 {
	MF_HAS_UV		= 0x1,
	MF_HAS_VELOCITY = 0x2,
	MF_HAS_MATERIAL = 0x4
};

class Normal;
class UV;
class Vertex;
struct FacePoint;
class Sampler;
class PR_LIB MeshContainer {
public:
	MeshContainer();
	~MeshContainer();

	inline void setVertices(const std::vector<float>& vx,
							const std::vector<float>& vy, const std::vector<float>& vz);
	inline const std::vector<float>& vertices(size_t dim) const { return mVertices[dim]; }

	inline void setNormals(const std::vector<float>& vx,
						   const std::vector<float>& vy, const std::vector<float>& vz);
	inline const std::vector<float>& normals(size_t dim) const { return mNormals[dim]; }

	inline void setUVs(const std::vector<float>& u,
					   const std::vector<float>& v);
	inline const std::vector<float>& uvs(size_t dim) const { return mUVs[dim]; }

	inline void setVelocities(const std::vector<float>& vx,
							  const std::vector<float>& vy, const std::vector<float>& vz);
	inline const std::vector<float>& velocities(size_t dim) const { return mVelocities[dim]; }

	inline void setIndices(const std::vector<uint32>& indices);
	inline const std::vector<uint32>& indices() const { return mIndices; }

	void setFaceVertexCount(const std::vector<uint8>& faceVertexCount);
	inline const std::vector<uint32>& faceOffset() const { return mFaceOffset; }
	inline size_t faceVertexCount(size_t face) const;

	inline void setMaterials(const std::vector<uint32>& f) { mMaterials = f; };
	inline const std::vector<uint32>& materials() const { return mMaterials; }

	inline uint32 features() const { return mFeatures; }

	inline size_t nodeCount() const { return mVertices[0].size(); }
	inline size_t triangleCount() const { return mTriangleCount; }
	inline size_t quadCount() const { return mQuadCount; }
	inline size_t faceCount() const { return triangleCount() + quadCount(); }

	inline Face getFace(uint32 index) const;

	float faceArea(size_t f, const Eigen::Affine3f& transform) const;
	float surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline bool isValid() const;

private:
	uint32 mFeatures;
	std::vector<float> mVertices[3];
	std::vector<float> mNormals[3];
	std::vector<float> mUVs[2];
	std::vector<float> mVelocities[3];
	std::vector<uint32> mMaterials;
	std::vector<uint32> mIndices;
	std::vector<uint32> mFaceOffset; // Only triangles and quads supported

	size_t mTriangleCount;
	size_t mQuadCount;
};
} // namespace PR

#include "MeshContainer.inl"