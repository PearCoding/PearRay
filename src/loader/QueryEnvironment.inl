namespace PR {

inline std::shared_ptr<PluginManager> QueryEnvironment::pluginManager() const { return mPluginManager; }
inline std::shared_ptr<CameraManager> QueryEnvironment::cameraManager() const { return mCameraManager; }
inline std::shared_ptr<EmissionManager> QueryEnvironment::emissionManager() const { return mEmissionManager; }
inline std::shared_ptr<EntityManager> QueryEnvironment::entityManager() const { return mEntityManager; }
inline std::shared_ptr<FilterManager> QueryEnvironment::filterManager() const { return mFilterManager; }
inline std::shared_ptr<InfiniteLightManager> QueryEnvironment::infiniteLightManager() const { return mInfiniteLightManager; }
inline std::shared_ptr<IntegratorManager> QueryEnvironment::integratorManager() const { return mIntegratorManager; }
inline std::shared_ptr<MaterialManager> QueryEnvironment::materialManager() const { return mMaterialManager; }
inline std::shared_ptr<NodeManager> QueryEnvironment::nodeManager() const { return mNodeManager; }
inline std::shared_ptr<SamplerManager> QueryEnvironment::samplerManager() const { return mSamplerManager; }
inline std::shared_ptr<SpectralMapperManager> QueryEnvironment::spectralMapperManager() const { return mSpectralMapperManager; }

} // namespace PR
