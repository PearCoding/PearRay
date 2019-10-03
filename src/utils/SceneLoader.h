#pragma once

#include "registry/Registry.h"
#include "shader/ShadingSocket.h"

#include <map>
#include <string>

namespace DL {
class Data;
class DataGroup;
} // namespace DL

namespace PR {
class VirtualEntity;
class IMesh;

class Environment;
class PR_LIB_UTILS SceneLoader {
	PR_CLASS_NON_CONSTRUCTABLE(SceneLoader);

public:
	static std::shared_ptr<Environment> loadFromFile(const std::string& wrkDir,
													 const std::string& path,
													 const std::string& pluginPath = "");
	static std::shared_ptr<Environment> loadFromString(const std::string& wrkDir,
													   const std::string& source,
													   const std::string& pluginPath = "");

	static Eigen::Vector3f getVector(const DL::DataGroup& arr, bool& ok);
	static Eigen::Matrix4f getMatrix(const DL::DataGroup& arr, bool& ok);
	static Eigen::Quaternionf getRotation(const DL::Data& data, bool& ok);

	static std::shared_ptr<FloatSpectralShadingSocket> getSpectralOutput(
		Environment* env, const DL::Data& data, bool allowScalar = false);
	static std::shared_ptr<FloatScalarShadingSocket> getScalarOutput(
		Environment* env, const DL::Data& data);
	static std::shared_ptr<FloatVectorShadingSocket> getVectorOutput(
		Environment* env, const DL::Data& data);

private:
	static void addRegistryEntry(const DL::DataGroup& group, Environment* env);
	static void addEntity(const DL::DataGroup& group,
						  const std::shared_ptr<VirtualEntity>& parent, Environment* env);
	static void addCamera(const DL::DataGroup& group, Environment* env);
	static void addLight(const DL::DataGroup& group, Environment* env);
	static void addEmission(const DL::DataGroup& group, Environment* env);
	static void addMaterial(const DL::DataGroup& group, Environment* env);
	static void addTexture(const DL::DataGroup& group, Environment* env);
	static void addSpectrum(const DL::DataGroup& group, Environment* env);
	static void addSubGraph(const DL::DataGroup& group, Environment* env);
	static void addMesh(const DL::DataGroup& group, Environment* env);

	static void addRegistryEntry(RegistryGroup regGroup, uint32 uuid, bool hasID,
								 const std::string& key, const DL::Data& group,
								 Environment* env);
	static void populateObjectRegistry(RegistryGroup regGroup, uint32 id,
									   const DL::DataGroup& group, Environment* env);
};
} // namespace PR
