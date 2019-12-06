#pragma once

#include "MeshContainer.h"

#include <vector>

namespace PR {
class kdTreeCollider;
struct GeometryPoint;

// Simple collidable mesh
class PR_LIB TriMesh {
public:
	TriMesh(const std::shared_ptr<MeshContainer>& mesh_container);
	~TriMesh();

	inline std::shared_ptr<MeshContainer> container() const { return mContainer; }

	float surfaceArea(uint32 id) const;
	bool isCollidable() const;
	float collisionCost() const;

	void build(const std::wstring& cnt_file);
	void load(const std::wstring& cnt_file);

	BoundingBox localBoundingBox() const;

	void checkCollision(const RayPackage& in, CollisionOutput& out) const;
	void checkCollision(const Ray& in, SingleCollisionOutput& out) const;

	Vector3f pickRandomParameterPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const;
	void provideGeometryPoint(uint32 faceID, const Vector3f& parameter, GeometryPoint& pt) const;

private:
	BoundingBox mBoundingBox;
	std::unique_ptr<kdTreeCollider> mKDTree;
	std::shared_ptr<MeshContainer> mContainer;
};
} // namespace PR
