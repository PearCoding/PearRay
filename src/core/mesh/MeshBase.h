#pragma once

#include "geometry/Face.h"
#include "mesh/MeshInfo.h"
#include "serialization/ISerializable.h"

#include <unordered_map>
#include <vector>

namespace PR {
class Normal;
class UV;
class Vertex;
struct FacePoint;
class ISampler;

enum class MeshComponent {
	Vertex = 0, // 3D
	Normal,		// 3D
	Texture,	// 2D
	Weight,		// 1D
	Velocity,	// 3D
	_COUNT
};

class PR_LIB_CORE MeshBase : public ISerializable {
public:
	inline static constexpr bool isComponent3D(MeshComponent component)
	{
		return component == MeshComponent::Vertex || component == MeshComponent::Normal || component == MeshComponent::Velocity;
	}
	inline static constexpr bool isComponent2D(MeshComponent component)
	{
		return component == MeshComponent::Texture;
	}
	inline static constexpr bool isComponent1D(MeshComponent component)
	{
		return component == MeshComponent::Weight;
	}

	MeshBase();
	virtual ~MeshBase();

	// ISerializable
	void serialize(Serializer& serializer) override;

	inline void setVertexComponent(MeshComponent component, const std::vector<float>& entries);
	inline void setVertexComponent(MeshComponent component, std::vector<float>&& entries);
	inline void setVertexComponentIndices(MeshComponent component, const std::vector<uint32>& indices);
	inline void setVertexComponentIndices(MeshComponent component, std::vector<uint32>&& indices);
	inline const std::vector<float>& vertexComponent(MeshComponent component) const { return mVertexComponents[(size_t)component].Entries; }
	inline const std::vector<uint32>& vertexComponentIndices(MeshComponent component) const { return mVertexComponents[(size_t)component].Indices; }
	inline Vector3f vertexComponent3D(MeshComponent component, size_t ind) const;
	inline Vector2f vertexComponent2D(MeshComponent component, size_t ind) const;
	inline float vertexComponent1D(MeshComponent component, size_t ind) const;

	inline bool hasVertexComponent(MeshComponent component) const { return !mVertexComponents[(size_t)component].Entries.empty(); }
	inline bool hasVertexComponentIndices(MeshComponent component) const { return !mVertexComponents[(size_t)component].Indices.empty(); }

	inline void assumeTriangular(size_t faceCount);
	inline void assumeQuadrangular(size_t faceCount);
	void setFaceVertexCount(const std::vector<uint8>& faceVertexCount);
	inline size_t faceVertexCount(size_t face) const;
	std::vector<uint32> faceVertexCounts() const;

	inline const std::vector<uint32>& faceIndexOffsets() const { return mFaceIndexOffset; }

	inline void setMaterialSlots(const std::vector<uint32>& f);
	inline void setMaterialSlots(std::vector<uint32>&& f);
	inline const std::vector<uint32>& materialSlots() const { return mMaterialSlots; }
	inline uint32 materialSlot(size_t index) const { return (features() & MeshFeature::Material) ? mMaterialSlots.at(index) : 0; }

	inline const MeshInfo& info() const { return mInfo; }
	inline MeshFeatures features() const { return mInfo.Features; }
	inline size_t nodeCount() const { return mInfo.NodeCount; }
	inline size_t triangleCount() const { return mInfo.TriangleCount; }
	inline size_t quadCount() const { return mInfo.QuadCount; }
	inline size_t faceCount() const { return triangleCount() + quadCount(); }
	inline bool isOnlyTriangular() const { return triangleCount() > 0 && quadCount() == 0; }
	inline bool isOnlyQuadrangular() const { return triangleCount() == 0 && quadCount() > 0; }

	inline Face getFace(uint32 index) const;

	size_t memoryFootprint() const;

	float faceArea(size_t f, const Eigen::Affine3f& transform) const;
	float surfaceArea(uint32 matID, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline bool isValid(std::string* errMsg = nullptr) const;

	BoundingBox constructBoundingBox() const;

	// Modifiers
	void buildSmoothNormals();
	void flipNormals();

private:
	inline uint32 faceIndexOffset(uint32 f) const;
	inline void handleInfo(MeshComponent component);

	struct VertexIndexPair {
		std::vector<float> Entries;
		std::vector<uint32> Indices;
	};

	MeshInfo mInfo;
	// Vertex components
	std::array<VertexIndexPair, (size_t)MeshComponent::_COUNT> mVertexComponents;
	// Face components
	std::vector<uint32> mMaterialSlots;

	std::vector<uint32> mFaceIndexOffset; // Only triangles and quads supported
};
} // namespace PR

#include "MeshBase.inl"