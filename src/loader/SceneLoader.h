#pragma once

#include "SceneLoadContext.h"

#include <filesystem>
#include <map>
#include <string>

namespace DL {
class Data;
class DataGroup;
} // namespace DL

namespace PR {
class ITransformable;
class IMesh;
class ParameterGroup;

class Environment;
class PR_LIB_LOADER SceneLoader {
	PR_CLASS_NON_CONSTRUCTABLE(SceneLoader);

public:
	struct LoadOptions {
		std::wstring WorkingDir;
		std::wstring PluginPath;
		uint32 CacheMode;
	};

	static std::shared_ptr<Environment> loadFromFile(const std::filesystem::path& path, const LoadOptions& opts);
	static std::shared_ptr<Environment> loadFromString(const std::string& source, const LoadOptions& opts);

private:
	static std::shared_ptr<Environment> createEnvironment(const std::vector<DL::DataGroup>& groups,
														  const LoadOptions& opts,
														  SceneLoadContext& ctx);
	static void setupEnvironment(const std::vector<DL::DataGroup>& groups, SceneLoadContext& ctx);
	static void setupTransformable(const DL::DataGroup& group,
								   const std::shared_ptr<PR::ITransformable>& entity, SceneLoadContext& ctx);
	static void addEntity(const DL::DataGroup& group,
						  const std::shared_ptr<ITransformable>& parent, SceneLoadContext& ctx);
	static void addSampler(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addFilter(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addIntegrator(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addCamera(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addLight(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addEmission(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addMaterial(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addTexture(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addSpectrum(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addSubGraph(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addInclude(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addMesh(const DL::DataGroup& group, SceneLoadContext& ctx);
	static void addNode(const DL::DataGroup& group, SceneLoadContext& ctx);
	static uint32 addNodeInline(const DL::DataGroup& group, SceneLoadContext& ctx);

	static ParameterGroup populateObjectParameters(const DL::DataGroup& group, SceneLoadContext& ctx);
	static Parameter unpackShadingNetwork(const DL::DataGroup& group, SceneLoadContext& ctx);

	static void include(const std::string& filename, SceneLoadContext& ctx);
};
} // namespace PR
