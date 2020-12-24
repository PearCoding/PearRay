#pragma once

#include "output/io/OutputSpecification.h"
#include "renderer/RenderSettings.h"
#include "shader/INode.h"
#include "spectral/ParametricBlob.h"

#include <list>
#include <map>
#include <utility>
#include <variant>

namespace PR {
class Cache;
class FrameOutputDevice;
class IEmission;
class IIntegrator;
class IMaterial;
class Parameter;
class RenderFactory;
class ResourceManager;
class SceneDatabase;
class ServiceObserver;
class SpectralUpsampler;

class CameraManager;
class EmissionManager;
class EntityManager;
class FilterManager;
class InfiniteLightManager;
class IntegratorManager;
class MaterialManager;
class NodeManager;
class PluginManager;
class SamplerManager;
class SpectralMapperManager;

class PR_LIB_LOADER BadRenderEnvironment : public std::exception {
public:
	const char* what() const throw()
	{
		return "Bad Render Environment";
	}
};

class PR_LIB_LOADER Environment {
public:
	inline static std::shared_ptr<Environment> createRenderEnvironment(const std::filesystem::path& workdir,
																	   const std::filesystem::path& plugdir,
																	   bool useStandardLib = true);

	inline static std::shared_ptr<Environment> createQueryEnvironment(const std::filesystem::path& plugdir,
																	  bool useStandardLib = true);

	virtual ~Environment();

	inline std::shared_ptr<ResourceManager> resourceManager() const;
	inline std::shared_ptr<Cache> cache() const;
	inline std::shared_ptr<SceneDatabase> sceneDatabase() const;

	inline void* textureSystem();

	inline void setWorkingDir(const std::filesystem::path& dir);
	inline std::filesystem::path workingDir() const;

	inline bool queryMode() const { return mQueryMode; }

	inline OutputSpecification& outputSpecification();
	inline const RenderSettings& renderSettings() const;
	inline RenderSettings& renderSettings();

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(RenderContext* renderer, FrameOutputDevice* outputDevice, ToneMapper& toneMapper, const OutputSaveOptions& options = OutputSaveOptions()) const;

	std::shared_ptr<IIntegrator> createSelectedIntegrator() const;
	std::shared_ptr<RenderFactory> createRenderFactory();
	std::shared_ptr<FrameOutputDevice> createAndAssignFrameOutputDevice(const std::shared_ptr<RenderContext>& context) const;

	inline std::shared_ptr<ServiceObserver> serviceObserver() const;

	inline std::shared_ptr<PluginManager> pluginManager() const;
	inline std::shared_ptr<CameraManager> cameraManager() const;
	inline std::shared_ptr<EmissionManager> emissionManager() const;
	inline std::shared_ptr<EntityManager> entityManager() const;
	inline std::shared_ptr<FilterManager> filterManager() const;
	inline std::shared_ptr<InfiniteLightManager> infiniteLightManager() const;
	inline std::shared_ptr<IntegratorManager> integratorManager() const;
	inline std::shared_ptr<MaterialManager> materialManager() const;
	inline std::shared_ptr<NodeManager> nodeManager() const;
	inline std::shared_ptr<SamplerManager> samplerManager() const;
	inline std::shared_ptr<SpectralMapperManager> spectralMapperManager() const;

	inline std::shared_ptr<SpectralUpsampler> defaultSpectralUpsampler() const { return mDefaultSpectralUpsampler; }

protected:
	Environment(const std::filesystem::path& workdir,
				const std::filesystem::path& plugdir,
				bool useStandardLib);

private:
	inline void getNode(const std::string& name, std::shared_ptr<FloatScalarNode>& node) const;
	inline void getNode(const std::string& name, std::shared_ptr<FloatSpectralNode>& node) const;
	inline void getNode(const std::string& name, std::shared_ptr<FloatVectorNode>& node) const;

	void initPlugins(const std::filesystem::path& pluginDir);

	std::filesystem::path mWorkingDir;
	RenderSettings mRenderSettings;

	const bool mQueryMode;
	const std::shared_ptr<ServiceObserver> mServiceObserver;

	std::shared_ptr<SceneDatabase> mSceneDatabase;

	// Order matters: PluginManager should be before other managers
	std::shared_ptr<PluginManager> mPluginManager;
	std::shared_ptr<CameraManager> mCameraManager;
	std::shared_ptr<EmissionManager> mEmissionManager;
	std::shared_ptr<EntityManager> mEntityManager;
	std::shared_ptr<FilterManager> mFilterManager;
	std::shared_ptr<InfiniteLightManager> mInfiniteLightManager;
	std::shared_ptr<IntegratorManager> mIntegratorManager;
	std::shared_ptr<MaterialManager> mMaterialManager;
	std::shared_ptr<NodeManager> mNodeManager;
	std::shared_ptr<SamplerManager> mSamplerManager;
	std::shared_ptr<SpectralMapperManager> mSpectralMapperManager;

	std::shared_ptr<SpectralUpsampler> mDefaultSpectralUpsampler;

	std::shared_ptr<ResourceManager> mResourceManager;
	std::shared_ptr<Cache> mCache;

	void* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR

#include "Environment.inl"