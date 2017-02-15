#pragma once

#include "RenderEntity.h"

#include <vector>

namespace PR
{
	class TriMesh;
	class Material;
	class PR_LIB MeshEntity : public RenderEntity
	{
	public:
		MeshEntity(uint32 id, const std::string& name);
		virtual ~MeshEntity();

		virtual std::string type() const;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setMesh(TriMesh* mesh);
		TriMesh* mesh() const;

		void reserveMaterialSlots(size_t count);
		void setMaterial(uint32 slot, Material* m);
		Material* material(uint32 slot) const;

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
		TriMesh* mMesh;
		std::vector<Material*> mMaterials;

		float mSurfaceArea_Cache;
	};
}
