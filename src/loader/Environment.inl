namespace PR {

inline std::shared_ptr<Environment> Environment::createRenderEnvironment(const std::filesystem::path& workdir,
																		 const std::filesystem::path& plugdir,
																		 bool useStandardLib)
{
	return std::shared_ptr<Environment>(new Environment(workdir, plugdir, useStandardLib));
}

inline std::shared_ptr<Environment> Environment::createQueryEnvironment(const std::filesystem::path& plugdir,
																		bool useStandardLib)
{
	return std::shared_ptr<Environment>(new Environment({}, plugdir, useStandardLib));
}

inline std::shared_ptr<ResourceManager> Environment::resourceManager() const { return mResourceManager; }
inline std::shared_ptr<SceneDatabase> Environment::sceneDatabase() const { return mSceneDatabase; }

inline void* Environment::textureSystem() { return mTextureSystem; }

inline void Environment::setWorkingDir(const std::filesystem::path& dir) { mWorkingDir = dir; }
inline std::filesystem::path Environment::workingDir() const { return mWorkingDir; }

inline OutputSpecification& Environment::outputSpecification() { return mOutputSpecification; }
inline const RenderSettings& Environment::renderSettings() const { return mRenderSettings; }
inline RenderSettings& Environment::renderSettings() { return mRenderSettings; }

inline std::shared_ptr<ServiceObserver> Environment::serviceObserver() const { return mServiceObserver; }
inline std::shared_ptr<PluginManager> Environment::pluginManager() const { return mPluginManager; }
inline std::shared_ptr<CameraManager> Environment::cameraManager() const { return mCameraManager; }
inline std::shared_ptr<EmissionManager> Environment::emissionManager() const { return mEmissionManager; }
inline std::shared_ptr<EntityManager> Environment::entityManager() const { return mEntityManager; }
inline std::shared_ptr<FilterManager> Environment::filterManager() const { return mFilterManager; }
inline std::shared_ptr<InfiniteLightManager> Environment::infiniteLightManager() const { return mInfiniteLightManager; }
inline std::shared_ptr<IntegratorManager> Environment::integratorManager() const { return mIntegratorManager; }
inline std::shared_ptr<MaterialManager> Environment::materialManager() const { return mMaterialManager; }
inline std::shared_ptr<NodeManager> Environment::nodeManager() const { return mNodeManager; }
inline std::shared_ptr<SamplerManager> Environment::samplerManager() const { return mSamplerManager; }
inline std::shared_ptr<SpectralMapperManager> Environment::spectralMapperManager() const { return mSpectralMapperManager; }

} // namespace PR
