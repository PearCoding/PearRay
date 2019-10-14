namespace PR {

inline std::shared_ptr<SpectrumDescriptor> Environment::spectrumDescriptor() const { return mSpectrumDescriptor; }
inline std::shared_ptr<PluginManager> Environment::pluginManager() const { return mPluginManager; }
inline std::shared_ptr<MaterialManager> Environment::materialManager() const { return mMaterialManager; }
inline std::shared_ptr<EntityManager> Environment::entityManager() const { return mEntityManager; }
inline std::shared_ptr<CameraManager> Environment::cameraManager() const { return mCameraManager; }
inline std::shared_ptr<EmissionManager> Environment::emissionManager() const { return mEmissionManager; }
inline std::shared_ptr<InfiniteLightManager> Environment::infiniteLightManager() const { return mInfiniteLightManager; }
inline std::shared_ptr<IntegratorManager> Environment::integratorManager() const { return mIntegratorManager; }

inline Spectrum Environment::getSpectrum(const std::string& name) const
{
	return mSpectrums.at(name);
}

inline bool Environment::hasSpectrum(const std::string& name) const
{
	return mSpectrums.count(name) != 0;
}

inline void Environment::addSpectrum(const std::string& name, const Spectrum& spec)
{
	mSpectrums.insert(std::make_pair(name, spec));
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
	mEmissions[name] = mat;
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
	mMaterials[name] = mat;
}

inline size_t Environment::materialCount() const
{
	return mMaterials.size();
}

inline std::shared_ptr<TriMesh> Environment::getMesh(const std::string& name) const
{
	return hasMesh(name) ? mMeshes.at(name) : nullptr;
}

inline bool Environment::hasMesh(const std::string& name) const
{
	return mMeshes.count(name) != 0;
}

inline void Environment::addMesh(const std::string& name, const std::shared_ptr<TriMesh>& m)
{
	PR_ASSERT(m, "Given mesh has to be valid");
	PR_ASSERT(!hasMesh(name), "Given name should be unique");
	mMeshes[name] = m;
}

inline void Environment::addShadingSocket(const std::string& name,
										  const ShadingSocketVariantPtr& output)
{
	//PR_ASSERT(output, "Given output has to be valid");
	PR_ASSERT(!hasShadingSocket(name), "Given name should be unique");
	mNamedShadingSocket[name] = output;
}

template <typename Socket>
inline std::shared_ptr<Socket> Environment::getShadingSocket(const std::string& name) const
{
	try {
		return boost::get<std::shared_ptr<Socket>>(mNamedShadingSocket.at(name));
	} catch (const boost::bad_get& e) {
		return std::shared_ptr<Socket>();
	}
}

inline bool Environment::hasShadingSocket(const std::string& name) const
{
	return mNamedShadingSocket.count(name) != 0;
}

template <typename Socket>
inline bool Environment::isShadingSocket(const std::string& name) const
{
	return hasShadingSocket(name) && getShadingSocket<Socket>(name);
}

inline OIIO::TextureSystem* Environment::textureSystem() { return mTextureSystem; }

inline void Environment::setWorkingDir(const std::string& dir) { mWorkingDir = dir; }
inline std::string Environment::workingDir() const { return mWorkingDir; }

inline OutputSpecification& Environment::outputSpecification() { return mOutputSpecification; }
inline const Registry& Environment::registry() const { return mRegistry; }
inline Registry& Environment::registry() { return mRegistry; }

} // namespace PR
