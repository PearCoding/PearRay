#pragma once

#include "RenderEntity.h"

#include <memory>
#include <vector>

namespace PR {
class TriMesh;
class Material;
class PR_LIB MeshEntity : public RenderEntity {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	MeshEntity(uint32 id, const std::string& name);
	virtual ~MeshEntity();

	std::string type() const override;

	bool isLight() const override;
	float surfaceArea(Material* m) const override;

	void setMesh(const std::shared_ptr<TriMesh>& mesh);
	const std::shared_ptr<TriMesh>& mesh() const;

	void reserveMaterialSlots(size_t count);
	void setMaterial(uint32 slot, const std::shared_ptr<Material>& m);
	std::shared_ptr<Material> material(uint32 slot) const;

	bool isCollidable() const override;
	float collisionCost() const override;
	BoundingBox localBoundingBox() const override;

	RenderEntity::Collision checkCollision(const Ray& ray) const override;
	RenderEntity::FacePointSample sampleFacePoint(Sampler& sampler, uint32 sample) const override;

	// Entity
	void onFreeze() override;
	std::string dumpInformation() const override;

	// RenderEntity
	void setup(RenderContext* context) override;

private:
	std::shared_ptr<TriMesh> mMesh;
	std::vector<std::shared_ptr<Material>> mMaterials;

	float mSurfaceArea_Cache;
};
}
