#pragma once

#include "RenderEntity.h"

namespace PR
{
	class Material;
	class PR_LIB CoordinateAxisEntity : public RenderEntity
	{
	public:
		CoordinateAxisEntity(uint32 id, const std::string& name);
		virtual ~CoordinateAxisEntity();

		virtual std::string type() const override;

		virtual bool isLight() const override;
		virtual float surfaceArea(Material* m) const override;

		void setAxisLength(float f);
		float axisLength() const;

		void setAxisThickness(float f);
		float axisThickness() const;

		void setXMaterial(Material* m);
		Material* xMaterial() const;

		void setYMaterial(Material* m);
		Material* yMaterial() const;

		void setZMaterial(Material* m);
		Material* zMaterial() const;

		virtual bool isCollidable() const override;
		virtual float collisionCost() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FaceSample& collisionPoint) const override;

		virtual FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

		// Entity
		virtual void onFreeze() override;

		// RenderEntity
		virtual void setup(RenderContext* context) override;

	private:
		void setup_cache() const;

		float mAxisLength;
		float mAxisThickness;

		Material* mMaterials[3];

		mutable BoundingBox mBoundingBox_Cache;
		mutable BoundingBox mAxisBoundingBox_Cache[3];
	};
}