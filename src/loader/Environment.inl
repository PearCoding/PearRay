namespace PR {

inline std::shared_ptr<ResourceManager> Environment::resourceManager() const { return mResourceManager; }
inline std::shared_ptr<Cache> Environment::cache() const { return mCache; }

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

inline void Environment::addNode(const std::string& name, const std::shared_ptr<INode>& output)
{
	PR_ASSERT(!hasNode(name), "Given name should be unique");
	mNamedNodes.emplace(name, output); // Skip default constructor
}

inline std::shared_ptr<INode> Environment::getRawNode(const std::string& name) const
{
	if (hasNode(name))
		return mNamedNodes.at(name);
	else
		return nullptr;
}

template <typename Socket>
inline std::shared_ptr<Socket> Environment::getNode(const std::string& name) const
{
	std::shared_ptr<Socket> node;
	getNode(name, node);
	return node;
}

inline void Environment::getNode(const std::string& name, std::shared_ptr<FloatScalarNode>& node2) const
{
	auto node = mNamedNodes.at(name);
	if (node->type() == NT_FloatScalar)
		node2 = std::reinterpret_pointer_cast<FloatScalarNode>(node);
	else
		node2 = nullptr;
}

inline void Environment::getNode(const std::string& name, std::shared_ptr<FloatSpectralNode>& node2) const
{
	auto node = mNamedNodes.at(name);
	if (node->type() == NT_FloatSpectral)
		node2 = std::reinterpret_pointer_cast<FloatSpectralNode>(node);
	else
		node2 = nullptr;
}

inline void Environment::getNode(const std::string& name, std::shared_ptr<FloatVectorNode>& node2) const
{
	auto node = mNamedNodes.at(name);
	if (node->type() == NT_FloatVector)
		node2 = std::reinterpret_pointer_cast<FloatVectorNode>(node);
	else
		node2 = nullptr;
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
