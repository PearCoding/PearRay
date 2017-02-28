#pragma once

#include "geometry/BoundingBox.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
	struct FaceSample;
	class IInfiniteLight;
	class RenderContext;
	class RenderEntity;
	class Ray;
	class PR_LIB Scene
	{
	public:
		explicit Scene(const std::string& name);
		virtual ~Scene();

		inline void setName(const std::string& name) { mName = name; }
		inline std::string name() const { return mName; }

		void addEntity(const std::shared_ptr<Entity>& e);
		void removeEntity(const std::shared_ptr<Entity>& e);
		void addEntity(const std::shared_ptr<RenderEntity>& e);
		void removeEntity(const std::shared_ptr<RenderEntity>& e);

		std::shared_ptr<Entity> getEntity(const std::string& name, const std::string& type) const;

		inline const std::list<std::shared_ptr<RenderEntity> >& renderEntities() const { return mRenderEntities; }
		inline const std::list<std::shared_ptr<Entity> >& entities() const { return mEntities; }

		void addInfiniteLight(const std::shared_ptr<IInfiniteLight>& e);
		void removeInfiniteLight(const std::shared_ptr<IInfiniteLight>& e);
		inline const std::list<std::shared_ptr<IInfiniteLight> >& infiniteLights() const { return mInfiniteLights; }

		void clear();

		void buildTree();

		RenderEntity* checkCollision(const Ray& ray, FaceSample& collisionPoint) const;
		bool checkIfCollides(const Ray& ray, FaceSample& collisionPoint) const;

		void freeze();
		void setup(RenderContext* context);

		BoundingBox boundingBox() const;
	private:
		std::string mName;
		std::list<std::shared_ptr<Entity> > mEntities;
		std::list<std::shared_ptr<RenderEntity> > mRenderEntities;
		std::list<std::shared_ptr<IInfiniteLight> > mInfiniteLights;

		void* mKDTree;// We use a void* pointer to hide the KDTree header only implementation
	};
}
