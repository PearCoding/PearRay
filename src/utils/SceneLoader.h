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

	class Environment;
	class PR_LIB_UTILS SceneLoader
	{
		PR_CLASS_NON_CONSTRUCTABLE(SceneLoader);
	public:
		static std::shared_ptr<Environment> loadFromFile(const std::string& path);
		static std::shared_ptr<Environment> loadFromString(const std::string& source);

		static Eigen::Vector3f getVector(const DL::DataGroup& arr, bool& ok);
		static Eigen::Quaternionf getRotation(const DL::Data& data, bool& ok);

		static std::shared_ptr<PR::SpectrumShaderOutput> getSpectralOutput(Environment* env, const DL::Data& data, bool allowScalar = false);
		static std::shared_ptr<PR::ScalarShaderOutput> getScalarOutput(Environment* env, const DL::Data& data);
		static std::shared_ptr<PR::VectorShaderOutput> getVectorOutput(Environment* env, const DL::Data& data);

	private:
		static void addEntity(const DL::DataGroup& group, const std::shared_ptr<PR::Entity>& parent, Environment* env);
		static void addLight(const DL::DataGroup& group, Environment* env);
		static void addMaterial(const DL::DataGroup& group, Environment* env);
		static void addTexture(const DL::DataGroup& group, Environment* env);
		static void addSpectrum(const DL::DataGroup& group, Environment* env);
		static void addSubGraph(const DL::DataGroup& group, Environment* env);
		static void addMesh(const DL::DataGroup& group, Environment* env);
	};
}
