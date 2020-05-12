#pragma once

#include "MeshBase.h"
#include "cache/ISerializeCachable.h"
#include "geometry/BoundingBox.h"

namespace PR {
class kdTreeCollider;
struct GeometryPoint;

class PR_LIB_CORE Mesh : public ISerializeCachable {
public:
	Mesh(const std::string& name,
		 std::unique_ptr<MeshBase>&& mesh_base,
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
	inline MeshBase* base_unsafe() const { return mBase.get(); }

protected:
	virtual void checkCollisionLocal(const RayPackage& in, CollisionOutput& out) = 0;
	virtual void checkCollisionLocal(const Ray& in, SingleCollisionOutput& out)  = 0;

	void serialize(Serializer& serializer) override;

	void beforeLoad() override;
	void afterLoad() override;
	void afterUnload() override;

protected:
	std::unique_ptr<kdTreeCollider> mCollider;
	std::unique_ptr<MeshBase> mBase;

private:
	void constructCollider();
	void loadCollider();

	void setup();

	BoundingBox mBoundingBox;
	MeshInfo mInfo;
};
} // namespace PR
