#pragma once

#include "parameter/ParameterGroup.h"
#include "spectral/SpectralBlob.h"

#include <filesystem>

namespace PR {
class Environment;
class FloatScalarNode;
class FloatSpectralNode;
class IEmission;
class IFilterFactory;
class IIntegratorFactory;
class INode;
class IMaterial;
class ISamplerFactory;
class ISpectralMapperFactory;
class ParameterGroup;

class PR_LIB_LOADER SceneLoadContext {
public:
	SceneLoadContext() = default;
	explicit SceneLoadContext(Environment* env);
	explicit SceneLoadContext(const std::filesystem::path& filename);
	SceneLoadContext(Environment* env, const std::filesystem::path& filename);

	bool hasFile(const std::filesystem::path& filename) const;
	void pushFile(const std::filesystem::path& filename);
	void popFile();

	inline std::filesystem::path currentFile() const
	{
		return mFileStack.empty() ? L"" : mFileStack.back();
	}

	inline std::filesystem::path escapePath(const std::filesystem::path& path) const
	{
		if (path.is_absolute())
			return path;

		if (std::filesystem::exists(path))
			return path;

		return std::filesystem::absolute(currentFile().parent_path() / path);
	}

	std::filesystem::path setupParametricImage(const std::filesystem::path& path);

	// Lookup functions for easier access
	std::shared_ptr<INode> lookupRawNode(const Parameter& parameter) const;
	std::shared_ptr<FloatSpectralNode> lookupSpectralNode(
		const Parameter& parameter, float def = 1) const;
	std::shared_ptr<FloatSpectralNode> lookupSpectralNode(
		const Parameter& parameter, const SpectralBlob& def) const;
	std::shared_ptr<FloatScalarNode> lookupScalarNode(
		const Parameter& parameter, float def = 1) const;

	std::shared_ptr<INode> lookupRawNode(const std::string& parameter) const;
	std::shared_ptr<FloatSpectralNode> lookupSpectralNode(
		const std::string& parameter, float def = 1) const;
	std::shared_ptr<FloatSpectralNode> lookupSpectralNode(
		const std::string& parameter, const SpectralBlob& def) const;
	std::shared_ptr<FloatScalarNode> lookupScalarNode(
		const std::string& parameter, float def = 1) const;

	std::shared_ptr<IMaterial> lookupMaterial(const Parameter& parameter) const;
	std::shared_ptr<IEmission> lookupEmission(const Parameter& parameter) const;

	inline const ParameterGroup& parameters() const { return mParameters; }
	inline ParameterGroup& parameters() { return mParameters; }

	inline const Transformf& transform() const { return mTransform; }
	inline Transformf& transform() { return mTransform; }

	inline void setEnvironment(Environment* env) { mEnvironment = env; }
	inline Environment* environment() const { return mEnvironment; }
	inline bool hasEnvironment() const { return mEnvironment != nullptr; }

	std::shared_ptr<IIntegratorFactory> loadIntegratorFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISamplerFactory> loadSamplerFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IFilterFactory> loadFilterFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISpectralMapperFactory> loadSpectralMapperFactory(const std::string& type, const ParameterGroup& params) const;

	std::shared_ptr<IMaterial> registerMaterial(const std::string& name, const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IMaterial> loadMaterial(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IEmission> registerEmission(const std::string& name, const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IEmission> loadEmission(const std::string& type, const ParameterGroup& params) const;

private:
	ParameterGroup mParameters;
	Transformf mTransform;
	std::vector<std::filesystem::path> mFileStack;
	Environment* mEnvironment = nullptr;
};
} // namespace PR
