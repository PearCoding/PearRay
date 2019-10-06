#pragma once

#include "geometry/BoundingBox.h"
#include "geometry/Face.h"
#include "geometry/FacePackage.h"
#include "geometry/Triangle.h"
#include "shader/ShadingPoint.h"

#include <Eigen/Geometry>
#include <vector>

namespace PR {
enum TriMeshFeatures : uint32 {
	TMF_HAS_UV		 = 0x1,
	TMF_HAS_VELOCITY = 0x2,
	TMF_HAS_MATERIAL = 0x4
};

class Face;
class Normal;
class UV;
class Vertex;
struct FacePoint;
class Sampler;
class PR_LIB TriMesh {
	PR_CLASS_NON_COPYABLE(TriMesh);

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	TriMesh();
	~TriMesh();

	inline void setVertices(const std::vector<float>& vx,
							const std::vector<float>& vy, const std::vector<float>& vz);
	inline void setNormals(const std::vector<float>& vx,
						   const std::vector<float>& vy, const std::vector<float>& vz);
	inline void setTangents(const std::vector<float>& vx,
							const std::vector<float>& vy, const std::vector<float>& vz);
	inline void setBitangents(const std::vector<float>& vx,
							  const std::vector<float>& vy, const std::vector<float>& vz);
	inline void setUVs(const std::vector<float>& u,
					   const std::vector<float>& v);
	inline void setVelocities(const std::vector<float>& vx,
							  const std::vector<float>& vy, const std::vector<float>& vz);
	inline void setIndices(const std::vector<uint32>& i1,
						   const std::vector<uint32>& i2, const std::vector<uint32>& i3);
	inline void setMaterials(const std::vector<uint32>& f) { mMaterials = f; };

	inline uint32 features() const { return mFeatures; }

	inline size_t nodeCount() const { return mVertices[0].size(); }
	inline size_t faceCount() const { return mIndices[0].size(); }

	inline Face getFace(uint32 index) const;
	inline uint32 getFaceMaterial(uint32 index) const;

	inline bool isValid() const;

	void clear();
	void build(const std::string& container_file, bool loadOnly = false);
	inline bool isBuilt() const { return mKDTree != nullptr; }

	float faceArea(uint32 f, const Eigen::Affine3f& transform) const;
	float surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline void setIntersectionTestCost(float f);
	inline float intersectionTestCost() const;

	inline const BoundingBox& boundingBox() const
	{
		return mBoundingBox;
	}

	float collisionCost() const;

	bool checkCollision(const Ray& in, SingleCollisionOutput& out) const;
	bool checkCollision(const RayPackage& in, CollisionOutput& out) const;

	void sampleFacePoint(float rnd1, float rnd2, float rnd3,
						 GeometryPoint& p, float& pdfA) const;

private:
	void buildTree(const std::string& container_file);
	void loadTree(const std::string& container_file);

	BoundingBox mBoundingBox;
	class kdTreeCollider* mKDTree;

	uint32 mFeatures;
	std::vector<float> mVertices[3];
	std::vector<float> mNormals[3];
	std::vector<float> mTangents[3];
	std::vector<float> mBitangents[3];
	std::vector<float> mUVs[2];
	std::vector<float> mVelocities[3];
	std::vector<uint32> mMaterials;
	std::vector<uint32> mIndices[3]; // Triangle corners

	float mIntersectionTestCost;
};
} // namespace PR

#include "TriMesh.inl"