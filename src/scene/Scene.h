#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	class FacePoint;
	class Ray;
	class Scene
	{
	public:
		Scene(const std::string& name);
		virtual ~Scene();

		void setName(const std::string& name);
		std::string name() const;

		void addEntity(Entity* e);
		void removeEntity(Entity* e);

		void clear();

		Entity* checkCollision(const Ray& ray, FacePoint& collisionPoint) const;

	private:
		std::string mName;
		std::list<Entity*> mEntities;
	};
}