#pragma once

#include "RenderEntity.h"

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

		void setMaterialOverride(Material* m);
		Material* materialOverride() const;

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

		virtual void onFreeze() override;
	private:
		TriMesh* mMesh;
		Material* mMaterialOverride;
		
		float mSurfaceArea_Cache;
	};
}