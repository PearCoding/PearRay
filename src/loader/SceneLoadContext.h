#pragma once

#include "parameter/ParameterGroup.h"
#include "spectral/SpectralBlob.h"

#include <filesystem>

namespace PR {
class Environment;
class FloatScalarNode;
class FloatSpectralNode;
class FloatVectorNode;
class IEmission;
class IFilterFactory;
class IIntegratorFactory;
class INode;
class IMaterial;
class ISamplerFactory;
class ISpectralMapperFactory;
class MeshBase;
class ParameterGroup;

class PR_LIB_LOADER SceneLoadContext {
public:
	SceneLoadContext(Environment* env, const std::filesystem::path& filename = {});

	bool hasFile(const std::filesystem::path& filename) const;
	void pushFile(const std::filesystem::path& filename);
	void popFile();

	inline std::filesystem::path currentFile() const
	{
		return mFileStack.empty() ? "" : mFileStack.back();
	}

	inline std::filesystem::path escapePath(const std::filesystem::path& path) const
	{
		// If absolute, nothing to escape
		if (path.is_absolute())
			return path;

		// If visible from the current working directory, use it
		if (std::filesystem::exists(path))
			return path;

		// Combine current file directory with given path
		return std::filesystem::absolute(currentFile().parent_path() / path);
	}

	std::filesystem::path setupParametricImage(const std::filesystem::path& path);

	// ---------------- Emission
	std::shared_ptr<IEmission> getEmission(const std::string& name) const;
	bool hasEmission(const std::string& name) const;
	uint32 addEmission(const std::string& name, const std::shared_ptr<IEmission>& mat);
	size_t emissionCount() const;

	// ---------------- Material
	std::shared_ptr<IMaterial> getMaterial(const std::string& name) const;
	bool hasMaterial(const std::string& name) const;
	uint32 addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat);
	size_t materialCount() const;

	// ---------------- Mesh
	std::shared_ptr<MeshBase> getMesh(const std::string& name) const;
	bool hasMesh(const std::string& name) const;
	void addMesh(const std::string& name, const std::shared_ptr<MeshBase>& m);

	// ---------------- Node
	uint32 addNode(const std::string& name, const std::shared_ptr<INode>& output);
	std::shared_ptr<INode> getRawNode(uint32 id) const;
	std::shared_ptr<INode> getRawNode(const std::string& name) const;
	bool hasNode(const std::string& name) const;

	template <typename Socket>
	inline std::shared_ptr<Socket> getNode(const std::string& name) const
	{
		std::shared_ptr<Socket> node;
		getNode(name, node);
		return node;
	}
	template <typename Socket>
	inline bool isNode(const std::string& name) const { return hasNode(name) && getNode<Socket>(name); }

	// ---------------- Lookup
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
	uint32 lookupMaterialID(const Parameter& parameter) const;
	std::vector<uint32> lookupMaterialIDArray(const Parameter& parameter, bool skipInvalid = false) const;
	std::shared_ptr<IMaterial> registerMaterial(const std::string& name, const std::string& type, const ParameterGroup& params);
	std::shared_ptr<IMaterial> loadMaterial(const std::string& type, const ParameterGroup& params) const;

	std::shared_ptr<IEmission> lookupEmission(const Parameter& parameter) const;
	uint32 lookupEmissionID(const Parameter& parameter) const;

	inline const ParameterGroup& parameters() const { return mParameters; }
	inline ParameterGroup& parameters() { return mParameters; }

	inline const Transformf& transform() const { return mTransform; }
	inline Transformf& transform() { return mTransform; }

	inline Environment* environment() const { return mEnvironment; }

	std::shared_ptr<IIntegratorFactory> loadIntegratorFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISamplerFactory> loadSamplerFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<IFilterFactory> loadFilterFactory(const std::string& type, const ParameterGroup& params) const;
	std::shared_ptr<ISpectralMapperFactory> loadSpectralMapperFactory(const std::string& type, const ParameterGroup& params) const;

	std::shared_ptr<IEmission> registerEmission(const std::string& name, const std::string& type, const ParameterGroup& params);
	std::shared_ptr<IEmission> loadEmission(const std::string& type, const ParameterGroup& params) const;

private:
	void getNode(const std::string& name, std::shared_ptr<FloatScalarNode>& node2) const;
	void getNode(const std::string& name, std::shared_ptr<FloatSpectralNode>& node2) const;
	void getNode(const std::string& name, std::shared_ptr<FloatVectorNode>& node2) const;

	ParameterGroup mParameters;
	Transformf mTransform;
	std::vector<std::filesystem::path> mFileStack;
	Environment* mEnvironment;

	std::unordered_map<std::string, std::shared_ptr<MeshBase>> mMeshes;
};
} // namespace PR
