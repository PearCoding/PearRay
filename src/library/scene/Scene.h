#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	class FacePoint;
	class GeometryEntity;
	class kdTree;
	class Ray;
	class PR_LIB Scene
	{
	public:
		Scene(const std::string& name);
		virtual ~Scene();

		void setName(const std::string& name);
		std::string name() const;

		void addEntity(GeometryEntity* e);
		void removeEntity(GeometryEntity* e);
		const std::list<GeometryEntity*>& geometryEntities() const;

		void addEntity(Entity* e);
		void removeEntity(Entity* e);
		Entity* getEntity(const std::string& name, const std::string& type) const;
		const std::list<Entity*>& entities() const;
		
		void clear();

		void buildTree();

		GeometryEntity* checkCollision(const Ray& ray, FacePoint& collisionPoint, Entity* ignore) const;

	private:
		std::string mName;
		std::list<Entity*> mEntities;
		std::list<GeometryEntity*> mGeometryEntities;

		kdTree* mKDTree;
	};
}