#pragma once

#include "BoundingBox.h"
#include "shader/FacePoint.h"
#include <Eigen/Geometry>
#include <vector>

namespace PR {
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

	void reserve(size_t count);
	void setFaces(const std::vector<Face*>& f);
	void addFace(Face* f);
	Face* getFace(size_t i) const;
	inline const std::vector<Face*>& faces() const
	{
		return mFaces;
	}

	void clear();

	void build();

	void calcNormals();

	float surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const;
	float surfaceArea(const Eigen::Affine3f& transform) const;

	inline const BoundingBox& boundingBox() const
	{
		return mBoundingBox;
	}

	float collisionCost() const;

	struct Collision {
		bool Successful;
		Face* Ptr;
		FacePoint Point;
	};
	Collision checkCollision(const Ray& ray);

	struct FacePointSample {
		FacePoint Point;
		uint32 MaterialSlot;
		float PDF;
	};
	FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd, uint32 sample) const;

private:
	BoundingBox mBoundingBox;
	void* mKDTree;

	float mPDF_Cache;
	std::vector<Face*> mFaces;
	std::vector<uint32> mMaterialSlots;
};
}
