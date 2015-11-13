#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>

namespace DL
{
	class DataArray;
	class DataGroup;
}

namespace PR
{
	class Entity;
}

namespace PRU
{
	class Environment;
	class PR_LIB_UTILS SceneLoader
	{
	public:
		SceneLoader();
		virtual ~SceneLoader();

		Environment* loadFromFile(const std::string& path);
		Environment* load(const std::string& source);

	private:
		void addEntity(DL::DataGroup* group, PR::Entity* parent, Environment* env);
		void addMaterial(DL::DataGroup* group, Environment* env);
		void addSpectrum(DL::DataGroup* group, Environment* env);
		void addMesh(DL::DataGroup* group, Environment* env);

		PM::vec3 getVector(DL::DataArray* arr, bool& ok) const;
	};
}
