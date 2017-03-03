#pragma once

#include "RenderEntity.h"

#include <vector>
#include <memory>

namespace PR
{
	class TriMesh;
	class Material;
	class PR_LIB MeshEntity : public RenderEntity
	{
	public:
		MeshEntity(uint32 id, const std::string& name);
		virtual ~MeshEntity();

		virtual std::string type() const override;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setMesh(const std::shared_ptr<TriMesh>& mesh);
		const std::shared_ptr<TriMesh>& mesh() const;

		void reserveMaterialSlots(size_t count);
		void setMaterial(uint32 slot, const std::shared_ptr<Material>& m);
		std::shared_ptr<Material> material(uint32 slot) const;

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

		// Entity
		virtual void onFreeze() override;
		virtual std::string dumpInformation() const override;

		// RenderEntity
		virtual void setup(RenderContext* context) override;

	private:
		std::shared_ptr<TriMesh> mMesh;
		std::vector<std::shared_ptr<Material> > mMaterials;

		float mSurfaceArea_Cache;
	};
}
