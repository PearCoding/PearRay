#pragma once

#include "Config.h"

#include <string>
#include <list>

namespace PR
{
	class Entity;
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
	private:
		std::string mName;
		std::list<Entity*> mEntities;
	};
}