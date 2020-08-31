#pragma once

#include "PR_Config.h"

#include <filesystem>

namespace PR {
class CameraManager;
class EmissionManager;
class EntityManager;
class FilterManager;
class InfiniteLightManager;
class IntegratorManager;
class MaterialManager;
class NodeManager;
class PluginManager;
class RenderFactory;
class SamplerManager;
class SpectralUpsampler;

// Special incomplete environment with read only access
class PR_LIB_LOADER QueryEnvironment {
public:
	explicit QueryEnvironment(const std::filesystem::path& plugdir);
	virtual ~QueryEnvironment();

	inline std::shared_ptr<PluginManager> pluginManager() const;
	inline std::shared_ptr<MaterialManager> materialManager() const;
	inline std::shared_ptr<EntityManager> entityManager() const;
	inline std::shared_ptr<CameraManager> cameraManager() const;
	inline std::shared_ptr<EmissionManager> emissionManager() const;
	inline std::shared_ptr<InfiniteLightManager> infiniteLightManager() const;
	inline std::shared_ptr<IntegratorManager> integratorManager() const;
	inline std::shared_ptr<FilterManager> filterManager() const;
	inline std::shared_ptr<SamplerManager> samplerManager() const;
	inline std::shared_ptr<NodeManager> nodeManager() const;

	inline std::shared_ptr<SpectralUpsampler> defaultSpectralUpsampler() const { return mDefaultSpectralUpsampler; }

protected:
	// Order matters: PluginManager should be before other managers
	std::shared_ptr<PluginManager> mPluginManager;
	std::shared_ptr<MaterialManager> mMaterialManager;
	std::shared_ptr<EntityManager> mEntityManager;
	std::shared_ptr<CameraManager> mCameraManager;
	std::shared_ptr<EmissionManager> mEmissionManager;
	std::shared_ptr<InfiniteLightManager> mInfiniteLightManager;
	std::shared_ptr<IntegratorManager> mIntegratorManager;
	std::shared_ptr<FilterManager> mFilterManager;
	std::shared_ptr<SamplerManager> mSamplerManager;
	std::shared_ptr<NodeManager> mNodeManager;

	std::shared_ptr<SpectralUpsampler> mDefaultSpectralUpsampler;

private:
	void loadPlugins(const std::filesystem::path& basedir);
};
} // namespace PR

#include "QueryEnvironment.inl"