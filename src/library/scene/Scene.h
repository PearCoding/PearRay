#pragma once

#include "geometry/BoundingBox.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	struct FaceSample;
	class IInfiniteLight;
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
		
		void addInfiniteLight(IInfiniteLight* e);
		void removeInfiniteLight(IInfiniteLight* e);
		inline const std::list<IInfiniteLight*>& infiniteLights() const { return mInfiniteLights; }

		void clear();

		void buildTree();

		RenderEntity* checkCollision(const Ray& ray, FaceSample& collisionPoint) const;
		bool checkIfCollides(const Ray& ray, FaceSample& collisionPoint) const;

		void freeze();

		BoundingBox boundingBox() const;
	private:
		std::string mName;
		std::list<Entity*> mEntities;
		std::list<RenderEntity*> mRenderEntities;
		std::list<IInfiniteLight*> mInfiniteLights;

		void* mKDTree;// We use a void* pointer to hide the KDTree header only implementation
	};
}
