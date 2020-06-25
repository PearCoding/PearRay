namespace PR {

inline std::shared_ptr<PluginManager> Environment::pluginManager() const { return mPluginManager; }
inline std::shared_ptr<MaterialManager> Environment::materialManager() const { return mMaterialManager; }
inline std::shared_ptr<EntityManager> Environment::entityManager() const { return mEntityManager; }
inline std::shared_ptr<CameraManager> Environment::cameraManager() const { return mCameraManager; }
inline std::shared_ptr<EmissionManager> Environment::emissionManager() const { return mEmissionManager; }
inline std::shared_ptr<InfiniteLightManager> Environment::infiniteLightManager() const { return mInfiniteLightManager; }
inline std::shared_ptr<IntegratorManager> Environment::integratorManager() const { return mIntegratorManager; }
inline std::shared_ptr<FilterManager> Environment::filterManager() const { return mFilterManager; }
inline std::shared_ptr<SamplerManager> Environment::samplerManager() const { return mSamplerManager; }
inline std::shared_ptr<ResourceManager> Environment::resourceManager() const { return mResourceManager; }
inline std::shared_ptr<Cache> Environment::cache() const { return mCache; }

inline ParametricBlob Environment::getSpectrum(const std::string& name) const
{
	return mSpectrums.at(name);
}

inline bool Environment::hasSpectrum(const std::string& name) const
{
	return mSpectrums.count(name) != 0;
}

inline void Environment::addSpectrum(const std::string& name, const ParametricBlob& spec)
{
	mSpectrums.emplace(name, spec);
}

inline std::shared_ptr<IEmission> Environment::getEmission(const std::string& name) const
{
	return hasEmission(name) ? mEmissions.at(name) : nullptr;
}

inline bool Environment::hasEmission(const std::string& name) const
{
	return mEmissions.count(name) != 0;
}

inline void Environment::addEmission(const std::string& name, const std::shared_ptr<IEmission>& mat)
{
	PR_ASSERT(mat, "Given emission has to be valid");
	PR_ASSERT(!hasEmission(name), "Given name should be unique");
	mEmissions.emplace(name, mat);
}

inline size_t Environment::emissionCount() const
{
	return mEmissions.size();
}

inline std::shared_ptr<IMaterial> Environment::getMaterial(const std::string& name) const
{
	return hasMaterial(name) ? mMaterials.at(name) : nullptr;
}

inline bool Environment::hasMaterial(const std::string& name) const
{
	return mMaterials.count(name) != 0;
}

inline void Environment::addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat)
{
	PR_ASSERT(mat, "Given material has to be valid");
	PR_ASSERT(!hasMaterial(name), "Given name should be unique");
	mMaterials.emplace(name, mat);
}

inline size_t Environment::materialCount() const
{
	return mMaterials.size();
}

inline std::shared_ptr<MeshBase> Environment::getMesh(const std::string& name) const
{
	return hasMesh(name) ? mMeshes.at(name) : nullptr;
}

inline bool Environment::hasMesh(const std::string& name) const
{
	return mMeshes.count(name) != 0;
}

inline void Environment::addMesh(const std::string& name, const std::shared_ptr<MeshBase>& m)
{
	PR_ASSERT(m, "Given mesh has to be valid");
	PR_ASSERT(!hasMesh(name), "Given name should be unique");
	mMeshes.emplace(name, m);
}

inline void Environment::addNode(const std::string& name,
										  const NodeVariantPtr& output)
{
	PR_ASSERT(!hasNode(name), "Given name should be unique");
	mNamedNodes.emplace(name, output);// Skip default constructor
}

template <typename Socket>
inline std::shared_ptr<Socket> Environment::getNode(const std::string& name) const
{
	try {
		return std::get<std::shared_ptr<Socket>>(mNamedNodes.at(name));
	} catch (const std::bad_variant_access&) {
		return nullptr;
	}
}

inline bool Environment::hasNode(const std::string& name) const
{
	return mNamedNodes.count(name) != 0;
}

template <typename Socket>
inline bool Environment::isNode(const std::string& name) const
{
	return hasNode(name) && getNode<Socket>(name);
}

inline void* Environment::textureSystem() { return mTextureSystem; }

inline void Environment::setWorkingDir(const std::wstring& dir) { mWorkingDir = dir; }
inline std::wstring Environment::workingDir() const { return mWorkingDir; }

inline OutputSpecification& Environment::outputSpecification() { return mOutputSpecification; }
inline const RenderSettings& Environment::renderSettings() const { return mRenderSettings; }
inline RenderSettings& Environment::renderSettings() { return mRenderSettings; }

} // namespace PR
