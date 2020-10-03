#pragma once

#include "parameter/ParameterGroup.h"
#include "spectral/SpectralBlob.h"

#include <filesystem>

namespace PR {
class Environment;
class FloatScalarNode;
class FloatSpectralNode;
class IFilterFactory;
class IIntegratorFactory;
class INode;
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

	inline const ParameterGroup& parameters() const { return mParameters; }
	inline ParameterGroup& parameters() { return mParameters; }

	inline void setEnvironment(Environment* env) { mEnvironment = env; }
	inline Environment* environment() const { return mEnvironment; }
	inline bool hasEnvironment() const { return mEnvironment != nullptr; }

	std::shared_ptr<IIntegratorFactory> loadIntegratorFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISamplerFactory> loadSamplerFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IFilterFactory> loadFilterFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISpectralMapperFactory> loadSpectralMapperFactory(const std::string& type, const ParameterGroup& params) const;

private:
	ParameterGroup mParameters;
	std::vector<std::filesystem::path> mFileStack;
	Environment* mEnvironment = nullptr;
};
} // namespace PR
