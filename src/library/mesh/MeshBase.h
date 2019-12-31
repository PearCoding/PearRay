#pragma once

#include "geometry/Face.h"
#include "mesh/MeshInfo.h"
#include "serialization/ISerializable.h"

#include <vector>

namespace PR {
class Normal;
class UV;
class Vertex;
struct FacePoint;
class ISampler;
class PR_LIB MeshBase : public ISerializable {
public:
	MeshBase();
	~MeshBase();

	// ISerializable
	void serialize(Serializer& serializer) override;

	inline void setVertices(const std::vector<float>& vertices);
	inline const std::vector<float>& vertices() const { return mVertices; }
	inline Vector3f vertex(size_t ind) const { return Vector3f(mVertices[3 * ind], mVertices[3 * ind + 1], mVertices[3 * ind + 2]); }

	inline void setNormals(const std::vector<float>& normals);
	inline const std::vector<float>& normals() const { return mNormals; }
	inline Vector3f normal(size_t ind) const { return Vector3f(mNormals[3 * ind], mNormals[3 * ind + 1], mNormals[3 * ind + 2]); }

	inline void setUVs(const std::vector<float>& uvs);
	inline const std::vector<float>& uvs() const { return mUVs; }
	inline Vector2f uv(size_t ind) const { return (features() & MF_HAS_UV) ? Vector2f(mUVs[2 * ind], mUVs[2 * ind + 1]) : Vector2f(0, 0); }

	inline void setVelocities(const std::vector<float>& velocities);
	inline const std::vector<float>& velocities() const { return mVelocities; }
	inline Vector3f velocity(size_t ind) const { return (features() & MF_HAS_VELOCITY) ? Vector3f(mVelocities[3 * ind], mVelocities[3 * ind + 1], mVelocities[3 * ind + 2]) : Vector3f(0, 0, 0); }

	inline void setIndices(const std::vector<uint32>& indices);
	inline const std::vector<uint32>& indices() const { return mIndices; }

	void setFaceVertexCount(const std::vector<uint8>& faceVertexCount);
	inline size_t faceVertexCount(size_t face) const;
	std::vector<uint32> faceVertexCounts() const;

	inline const std::vector<uint32>& faceIndexOffsets() const { return mFaceIndexOffset; }

	inline void setMaterialSlots(const std::vector<uint32>& f);
	inline const std::vector<uint32>& materialSlots() const { return mMaterialSlots; }
	inline uint32 materialSlot(size_t index) const { return (features() & MF_HAS_MATERIAL) ? mMaterialSlots.at(index) : 0; }

	inline const MeshInfo& info() const { return mInfo; }
	inline uint32 features() const { return mInfo.Features; }
	inline size_t nodeCount() const { return mInfo.NodeCount; }
	inline size_t triangleCount() const { return mInfo.TriangleCount; }
	inline size_t quadCount() const { return mInfo.QuadCount; }
	inline size_t faceCount() const { return triangleCount() + quadCount(); }
	inline bool isOnlyTriangular() const { return triangleCount() > 0 && quadCount() == 0; }
	inline bool isOnlyQuadrangular() const { return triangleCount() == 0 && quadCount() > 0; }

	inline Face getFace(uint32 index) const;

	float faceArea(size_t f, const Eigen::Affine3f& transform) const;
	float surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline bool isValid() const;

	BoundingBox constructBoundingBox() const;

	size_t addUserVertexAttrib(const std::vector<float>& cnt);
	inline std::vector<float>& userVertexAttrib(size_t id) { return mUserVertexAttribs.at(id); }
	size_t addUserFaceAttrib(const std::vector<float>& cnt);
	inline std::vector<float>& userFaceAttrib(size_t id) { return mUserFaceAttribs.at(id); }

	// Modifiers
	void buildNormals();
	void flipNormals();
	void triangulate();

private:
	MeshInfo mInfo;
	std::vector<float> mVertices;   //3 floats
	std::vector<float> mNormals;	//3 floats
	std::vector<float> mUVs;		// 2 floats
	std::vector<float> mVelocities; //3 floats
	std::vector<uint32> mMaterialSlots;
	std::vector<uint32> mIndices;
	std::vector<uint32> mFaceIndexOffset; // Only triangles and quads supported

	std::vector<std::vector<float>> mUserVertexAttribs;
	std::vector<std::vector<float>> mUserFaceAttribs;
};
} // namespace PR

#include "MeshBase.inl"