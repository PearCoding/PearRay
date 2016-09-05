#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	struct SamplePoint;
	class RenderEntity;
	class Ray;
	class PR_LIB Scene
	{
	public:
		explicit Scene(const std::string& name);
		virtual ~Scene();

		inline void setName(const std::string& name) { mName = name; }
		inline std::string name() const { return mName; }

		inline const std::list<RenderEntity*>& renderEntities() const { return mRenderEntities; }

		void addEntity(Entity* e);
		void removeEntity(Entity* e);
		Entity* getEntity(const std::string& name, const std::string& type) const;
		inline const std::list<Entity*>& entities() const { return mEntities; }
		
		void clear();

		void buildTree();

		RenderEntity* checkCollision(const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore) const;

		void onPreRender();
	private:
		std::string mName;
		std::list<Entity*> mEntities;
		std::list<RenderEntity*> mRenderEntities;

		void* mKDTree;
	};
}
