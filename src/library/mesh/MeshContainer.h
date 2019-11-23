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
	inline Vector3f vertex(size_t ind) const { return Vector3f(mVertices[0][ind], mVertices[1][ind], mVertices[2][ind]); }

	inline void setNormals(const std::vector<float>& vx,
						   const std::vector<float>& vy, const std::vector<float>& vz);
	inline const std::vector<float>& normals(size_t dim) const { return mNormals[dim]; }
	inline Vector3f normal(size_t ind) const { return Vector3f(mNormals[0][ind], mNormals[1][ind], mNormals[2][ind]); }

	inline void setUVs(const std::vector<float>& u,
					   const std::vector<float>& v);
	inline const std::vector<float>& uvs(size_t dim) const { return mUVs[dim]; }
	inline Vector2f uv(size_t ind) const { return (features() & MF_HAS_UV) ? Vector2f(mUVs[0][ind], mUVs[1][ind]) : Vector2f(0, 0); }

	inline void setVelocities(const std::vector<float>& vx,
							  const std::vector<float>& vy, const std::vector<float>& vz);
	inline const std::vector<float>& velocities(size_t dim) const { return mVelocities[dim]; }
	inline Vector3f velocity(size_t ind) const { return Vector3f(mVelocities[0][ind], mVelocities[1][ind], mVelocities[2][ind]); }

	inline void setIndices(const std::vector<uint32>& indices);
	inline const std::vector<uint32>& indices() const { return mIndices; }

	void setFaceVertexCount(const std::vector<uint8>& faceVertexCount);
	inline size_t faceVertexCount(size_t face) const;
	std::vector<uint32> faceVertexCounts() const;

	inline const std::vector<uint32>& faceIndexOffsets() const { return mFaceIndexOffset; }

	inline void setMaterialSlots(const std::vector<uint32>& f);
	inline const std::vector<uint32>& materialSlots() const { return mMaterialSlots; }
	inline uint32 materialSlot(size_t index) const { return (features() & MF_HAS_MATERIAL) ? mMaterialSlots.at(index) : 0; }

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

	BoundingBox constructBoundingBox() const;
	void triangulate();

private:
	uint32 mFeatures;
	std::vector<float> mVertices[3];
	std::vector<float> mNormals[3];
	std::vector<float> mUVs[2];
	std::vector<float> mVelocities[3];
	std::vector<uint32> mMaterialSlots;
	std::vector<uint32> mIndices;
	std::vector<uint32> mFaceIndexOffset; // Only triangles and quads supported

	size_t mTriangleCount;
	size_t mQuadCount;
};
} // namespace PR

#include "MeshContainer.inl"