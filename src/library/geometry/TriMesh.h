#pragma once

#include "BoundingBox.h"
#include "Vector.h"
#include "Face.h"
#include "Triangle.h"
#include "shader/FacePoint.h"
#include <Eigen/Geometry>
#include <vector>

namespace PR {
enum TriMeshFeatures : uint32 {
	TMF_HAS_UV = 0x1,
	TMF_HAS_VELOCITY = 0x2,
	TMF_HAS_MATERIAL = 0x4
};

class Face;
class Normal;
class UV;
class Vertex;
class Material;
class Ray;
struct FacePoint;
class Sampler;
class PR_LIB TriMesh {
	PR_CLASS_NON_COPYABLE(TriMesh);

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	TriMesh();
	~TriMesh();

	inline void setVertices(const std::vector<Vector3f>& f) { mVertices = f; }
	inline void setNormals(const std::vector<Vector3f>& f) { mNormals = f; };
	inline void setTangents(const std::vector<Vector3f>& f) { mTangents = f; };
	inline void setBitangents(const std::vector<Vector3f>& f) { mBitangents = f; };
	inline void setUVs(const std::vector<Vector2f>& f) { mUVs = f; };
	inline void setVelocities(const std::vector<Vector3f>& f) { mVelocities = f; };
	inline void setMaterials(const std::vector<uint32>& f) { mMaterials = f; };
	inline void setIndices(const std::vector<Vector3u64>& f) { mIndices = f; };

	inline uint32 features() const { return mFeatures; }

	inline size_t nodeCount() const { return mVertices.size(); }
	inline size_t faceCount() const { return mIndices.size(); }
	
	inline Face getFace(uint64 index) const;
	inline uint32 getFaceMaterial(uint64 index) const;

	inline bool isValid() const;

	void clear();
	void build(const std::string& container_file, bool loadOnly=false);
	inline bool isBuilt() const { return mKDTree != nullptr; }

	float surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline void setIntersectionTestCost(float f);
	inline float intersectionTestCost() const;

	inline const BoundingBox& boundingBox() const
	{
		return mBoundingBox;
	}

	float collisionCost() const;

	struct Collision {
		bool Successful;
		uint64 Index;
		FacePoint Point;
	};
	Collision checkCollision(const Ray& ray);

	struct FacePointSample {
		FacePoint Point;
		uint32 MaterialSlot;
		float PDF_A;
	};
	FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd) const;

private:
	void buildTree(const std::string& container_file);
	void loadTree(const std::string& container_file);

	BoundingBox mBoundingBox;
	class kdTreeCollider* mKDTree;

	uint32 mFeatures;
	std::vector<Vector3f> mVertices;
	std::vector<Vector3f> mNormals;
	std::vector<Vector3f> mTangents;
	std::vector<Vector3f> mBitangents;
	std::vector<Vector2f> mUVs;
	std::vector<Vector3f> mVelocities;
	std::vector<uint32> mMaterials;
	std::vector<Vector3u64> mIndices;

	float mIntersectionTestCost;
};
}

#include "TriMesh.inl"