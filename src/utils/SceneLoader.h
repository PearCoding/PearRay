#pragma once

#include "shader/ShaderOutput.h"

#include <map>
#include <string>

namespace DL
{
	class Data;
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

		PM::vec3 getVector(const DL::DataGroup& arr, bool& ok) const;
		PM::quat getRotation(const DL::Data& data, bool& ok) const;

		PR::SpectralShaderOutput* getSpectralOutput(Environment* env, const DL::Data& data, bool allowScalar = false) const;
		PR::ScalarShaderOutput* getScalarOutput(Environment* env, const DL::Data& data) const;
		PR::VectorShaderOutput* getVectorOutput(Environment* env, const DL::Data& data) const;

	private:
		void addEntity(const DL::DataGroup& group, PR::Entity* parent, Environment* env);
		void addLight(const DL::DataGroup& group, Environment* env);
		void addMaterial(const DL::DataGroup& group, Environment* env);
		void addTexture(const DL::DataGroup& group, Environment* env);
		void addSpectrum(const DL::DataGroup& group, Environment* env);
		void addSubGraph(const DL::DataGroup& group, Environment* env);
		void addMesh(const DL::DataGroup& group, Environment* env);
	};
}
