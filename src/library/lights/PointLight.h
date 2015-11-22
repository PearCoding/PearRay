#pragma once

#include "entity/RenderEntity.h"

namespace PR
{
	class Material;
	class PR_LIB PointLight : public RenderEntity
	{
	public:
		PointLight(const std::string& name, Entity* parent = nullptr);
		virtual ~PointLight();

		virtual std::string type() const;

		bool isLight() const;
		uint32 maxLightSamples() const override;

		void setMaterial(Material* m);
		Material* material() const;

		virtual bool isCollidable() const override;
		virtual BoundingBox localBoundingBox() const override;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint) override;

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer) override;

		virtual FacePoint getRandomFacePoint(Random& random) const;

	private:
		Material* mMaterial;
	};
}