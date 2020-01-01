#pragma once

#include "SceneLoadContext.h"
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
	struct LoadOptions {
		std::wstring WorkingDir;
		std::wstring PluginPath;
		uint32 CacheMode;
	};

	static std::shared_ptr<Environment> loadFromFile(const std::wstring& path, const LoadOptions& opts);
	static std::shared_ptr<Environment> loadFromString(const std::string& source, const LoadOptions& opts);

private:
	static std::shared_ptr<Environment> createEnvironment(const std::vector<DL::DataGroup>& groups,
														  const LoadOptions& opts,
														  SceneLoadContext& ctx);
	static void setupEnvironment(const std::vector<DL::DataGroup>& groups, SceneLoadContext& ctx);
	static void addRegistryEntry(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void setupTransformable(const DL::DataGroup& group,
								   const std::shared_ptr<PR::ITransformable>& entity, SceneLoadContext& ctx);
	static void addEntity(const DL::DataGroup& group,
						  const std::shared_ptr<ITransformable>& parent, SceneLoadContext& ctx);
	static void addCamera(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addLight(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addEmission(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addMaterial(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addTexture(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addSpectrum(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addSubGraph(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addInclude(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addMesh(const DL::DataGroup& group, SceneLoadContext& ctx);

	static void addRegistryEntry(RegistryGroup regGroup, uint32 uuid, bool hasID,
								 const std::string& key, const DL::Data& group,
								 SceneLoadContext& ctx);
	static void populateObjectRegistry(RegistryGroup regGroup, uint32 id,
									   const DL::DataGroup& group, SceneLoadContext& ctx);

	static void include(const std::string& filename, SceneLoadContext& ctx);
};
} // namespace PR
