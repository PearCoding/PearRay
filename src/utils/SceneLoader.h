#pragma once

#include "registry/Registry.h"

#include <map>
#include <string>

namespace DL {
class Data;
class DataGroup;
} // namespace DL

namespace PR {
class ITransformable;
class IMesh;

class Environment;
class PR_LIB_UTILS SceneLoader {
	PR_CLASS_NON_CONSTRUCTABLE(SceneLoader);

public:
	static std::shared_ptr<Environment> loadFromFile(const std::wstring& wrkDir,
													 const std::wstring& path,
													 const std::wstring& pluginPath = L"");
	static std::shared_ptr<Environment> loadFromString(const std::wstring& wrkDir,
													   const std::string& source,
													   const std::wstring& pluginPath = L"");

private:
	static void addRegistryEntry(const DL::DataGroup& group, Environment* env);
	static void setupTransformable(const DL::DataGroup& group,
								   const std::shared_ptr<PR::ITransformable>& entity, Environment* env);
	static void addEntity(const DL::DataGroup& group,
						  const std::shared_ptr<ITransformable>& parent, Environment* env);
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
