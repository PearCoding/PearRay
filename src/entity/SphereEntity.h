#pragma once

#include "Entity.h"

namespace PR
{
	class Material;
	class SphereEntity : public Entity
	{
	public:
		SphereEntity(const std::string& name, float r, Entity* parent = nullptr);
		virtual ~SphereEntity();

		void setRadius(float f);
		float radius() const;

		void setMaterial(Material* m);
		Material* material() const;

		virtual bool checkCollision(const Ray& ray, PM::vec3& collisionPoint);

	private:
		float mRadius;
		Material* mMaterial;
	};
}