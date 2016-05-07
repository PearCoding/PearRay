#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	class FacePoint;
	class RenderEntity;
	class kdTree;
	class Ray;
	class PR_LIB Scene
	{
	public:
		explicit Scene(const std::string& name);
		virtual ~Scene();

		void setName(const std::string& name);
		std::string name() const;

		const std::list<RenderEntity*>& renderEntities() const;

		void addEntity(Entity* e);
		void removeEntity(Entity* e);
		Entity* getEntity(const std::string& name, const std::string& type) const;
		const std::list<Entity*>& entities() const;
		
		void clear();

		void buildTree();

		RenderEntity* checkCollision(const Ray& ray, FacePoint& collisionPoint, Entity* ignore) const;

	private:
		std::string mName;
		std::list<Entity*> mEntities;
		std::list<RenderEntity*> mRenderEntities;

		kdTree* mKDTree;
	};
}