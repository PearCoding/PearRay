#pragma once

#include "PR_Config.h"

#include <string>

namespace PR {
class Registry;
class EntityManager;
class PluginManager;
class MaterialManager;
class CameraManager;
class LightManager;
class InfiniteLightManager;
class IntegratorManager;
class Scene;
class RenderFactory;
class SpectrumDescriptor;
class PR_LIB RenderManager {
public:
	RenderManager(const std::string& workingDir);
	virtual ~RenderManager();

	inline std::shared_ptr<Registry> registry() const { return mRegistry; }
	inline std::shared_ptr<PluginManager> pluginManager() const { return mPluginManager; }
	inline std::shared_ptr<MaterialManager> materialManager() const { return mMaterialManager; }
	inline std::shared_ptr<EntityManager> entityManager() const { return mEntityManager; }
	inline std::shared_ptr<CameraManager> cameraManager() const { return mCameraManager; }
	inline std::shared_ptr<LightManager> lightManager() const { return mLightManager; }
	inline std::shared_ptr<InfiniteLightManager> infiniteLightManager() const { return mInfiniteLightManager; }
	/*inline std::shared_ptr<IntegratorManager> integratorManager() const { return mIntegratorManager; }*/

	inline void setSpectrumDescriptor(const std::shared_ptr<SpectrumDescriptor>& desc)
	{
		mSpectrumDescriptor = desc;
	}
	inline std::shared_ptr<SpectrumDescriptor> spectrumDescriptor() const
	{
		return mSpectrumDescriptor;
	}

	inline void setWorkingDir(const std::string& dir) { mWorkingDir = dir; }
	inline std::string workingDir() const { return mWorkingDir; }

	std::shared_ptr<Scene> createScene() const;
	std::shared_ptr<RenderFactory> createRenderFactory() const;

private:
	std::string mWorkingDir;

	// Order matters: PluginManager should be before other managers
	std::shared_ptr<Registry> mRegistry;
	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;
	std::shared_ptr<PluginManager> mPluginManager;
	std::shared_ptr<MaterialManager> mMaterialManager;
	std::shared_ptr<EntityManager> mEntityManager;
	std::shared_ptr<CameraManager> mCameraManager;
	std::shared_ptr<LightManager> mLightManager;
	std::shared_ptr<InfiniteLightManager> mInfiniteLightManager;
	/*std::shared_ptr<IntegratorManager> mIntegratorManager;*/
};
} // namespace PR
