#pragma once

#include "MeshBase.h"
#include "cache/ISerializeCachable.h"
#include "geometry/BoundingBox.h"

namespace PR {
class kdTreeCollider;
struct GeometryPoint;

class PR_LIB Mesh : public ISerializeCachable {
public:
	Mesh(const std::string& name,
		 const std::shared_ptr<MeshBase>& mesh_base,
		 const std::shared_ptr<Cache>& cache,
		 bool useCache);
	virtual ~Mesh();

	float surfaceArea(uint32 id);
	bool isCollidable() const;
	float collisionCost() const;

	BoundingBox localBoundingBox() const;

	void checkCollision(const RayPackage& in, CollisionOutput& out);
	void checkCollision(const Ray& in, SingleCollisionOutput& out);

	Vector3f pickRandomParameterPoint(const Vector2f& rnd, uint32& faceID, float& pdf);
	void provideGeometryPoint(uint32 faceID, const Vector3f& parameter, GeometryPoint& pt);

	// Use this only when sure it is loaded
	inline std::shared_ptr<MeshBase> base_unsafe() const { return mBase; }

protected:
	virtual void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) = 0;
	virtual void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)  = 0;

	void serialize(Serializer& serializer);

	void beforeLoad() override;
	void afterLoad() override;
	void afterUnload() override;

protected:
	std::shared_ptr<kdTreeCollider> mCollider;
	std::shared_ptr<MeshBase> mBase;

private:
	void constructCollider();
	void loadCollider();

	void setup();

	BoundingBox mBoundingBox;
	MeshInfo mInfo;
};
} // namespace PR
