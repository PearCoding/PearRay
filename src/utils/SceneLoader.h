#pragma once

#include "Config.h"
#include "PearMath.h"

#include "shader/ShaderOutput.h"

#include <map>
#include <string>

namespace DL
{
	class Data;
	class DataArray;
	class DataGroup;
}

namespace PR
{
	class Entity;
	class IMesh;
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

		PM::vec3 getVector(DL::DataArray* arr, bool& ok) const;
		PM::quat getRotation(DL::Data* data, bool& ok) const;

		PR::SpectralShaderOutput* getSpectralOutput(Environment* env, DL::Data* data) const;
		PR::ScalarShaderOutput* getScalarOutput(Environment* env, DL::Data* data) const;
		PR::VectorShaderOutput* getVectorOutput(Environment* env, DL::Data* data) const;

	private:
		void addEntity(DL::DataGroup* group, PR::Entity* parent, Environment* env);
		void addMaterial(DL::DataGroup* group, Environment* env);
		void addSpectrum(DL::DataGroup* group, Environment* env);
		void addSubGraph(DL::DataGroup* group, Environment* env);
	};
}
