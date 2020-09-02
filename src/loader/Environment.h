#pragma once

#include "QueryEnvironment.h"
#include "output/OutputSpecification.h"
#include "renderer/RenderSettings.h"
#include "shader/INode.h"
#include "spectral/ParametricBlob.h"

#include <list>
#include <map>
#include <utility>
#include <variant>

namespace PR {
class Cache;
class IEmission;
class IIntegrator;
class IMaterial;
class MeshBase;
class Parameter;
class RenderFactory;
class ResourceManager;

class PR_LIB_LOADER BadRenderEnvironment : public std::exception {
public:
	const char* what() const throw()
	{
		return "Bad Render Environment";
	}
};

class PR_LIB_LOADER Environment : public QueryEnvironment {
public:
	Environment(const std::wstring& workdir,
				const std::wstring& plugdir,
				bool useStandardLib = true);
	virtual ~Environment();

	inline std::shared_ptr<ResourceManager> resourceManager() const;
	inline std::shared_ptr<Cache> cache() const;

	inline std::shared_ptr<IEmission> getEmission(const std::string& name) const;
	inline bool hasEmission(const std::string& name) const;
	inline void addEmission(const std::string& name, const std::shared_ptr<IEmission>& mat);
	inline size_t emissionCount() const;

	inline std::shared_ptr<IMaterial> getMaterial(const std::string& name) const;
	inline bool hasMaterial(const std::string& name) const;
	inline void addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat);
	inline size_t materialCount() const;

	inline std::shared_ptr<MeshBase> getMesh(const std::string& name) const;
	inline bool hasMesh(const std::string& name) const;
	inline void addMesh(const std::string& name, const std::shared_ptr<MeshBase>& m);

	inline void addNode(const std::string& name, const std::shared_ptr<INode>& output);
	std::shared_ptr<INode> getRawNode(uint64 id) const;
	inline std::shared_ptr<INode> getRawNode(const std::string& name) const;
	template <typename Socket>
	inline std::shared_ptr<Socket> getNode(const std::string& name) const;
	inline bool hasNode(const std::string& name) const;
	template <typename Socket>
	inline bool isNode(const std::string& name) const;

	inline void* textureSystem();

	inline void setWorkingDir(const std::wstring& dir);
	inline std::wstring workingDir() const;

	inline OutputSpecification& outputSpecification();
	inline const RenderSettings& renderSettings() const;
	inline RenderSettings& renderSettings();

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(RenderContext* renderer, ToneMapper& toneMapper, const OutputSaveOptions& options = OutputSaveOptions()) const;

	std::shared_ptr<IIntegrator> createSelectedIntegrator() const;
	std::shared_ptr<RenderFactory> createRenderFactory();

private:
	inline void getNode(const std::string& name, std::shared_ptr<FloatScalarNode>& node) const;
	inline void getNode(const std::string& name, std::shared_ptr<FloatSpectralNode>& node) const;
	inline void getNode(const std::string& name, std::shared_ptr<FloatVectorNode>& node) const;

	std::wstring mWorkingDir;
	RenderSettings mRenderSettings;

	std::shared_ptr<ResourceManager> mResourceManager;
	std::shared_ptr<Cache> mCache;

	std::map<std::string, std::shared_ptr<IEmission>> mEmissions;
	std::map<std::string, std::shared_ptr<IMaterial>> mMaterials;
	std::map<std::string, std::shared_ptr<MeshBase>> mMeshes;

	std::map<std::string, std::shared_ptr<INode>> mNamedNodes;

	void* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR

#include "Environment.inl"