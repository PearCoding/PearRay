#pragma once

#include "output/OutputSpecification.h"
#include "renderer/RenderSettings.h"
#include "shader/Socket.h"
#include "spectral/ParametricBlob.h"

#include <variant>
#include <list>
#include <map>
#include <utility>

namespace PR {
class IEmission;
class IMaterial;
class MeshBase;
class PluginManager;
class MaterialManager;
class EmissionManager;
class EntityManager;
class CameraManager;
class InfiniteLightManager;
class IntegratorManager;
class FilterManager;
class SamplerManager;
class IIntegrator;
class RenderFactory;
class ResourceManager;
class Cache;
class Parameter;
class SpectralUpsampler;

using ShadingSocketVariantPtr = std::variant<
	std::monostate,
	std::shared_ptr<FloatScalarShadingSocket>,
	std::shared_ptr<FloatSpectralShadingSocket>,
	std::shared_ptr<FloatVectorShadingSocket>>;

class PR_LIB_LOADER BadRenderEnvironment : public std::exception {
public:
	const char* what() const throw()
	{
		return "Bad Render Environment";
	}
};

class PR_LIB_LOADER Environment {
public:
	explicit Environment(const std::wstring& workdir,
						 const std::wstring& plugdir,
						 bool useStandardLib = true);
	virtual ~Environment();

	inline std::shared_ptr<PluginManager> pluginManager() const;
	inline std::shared_ptr<MaterialManager> materialManager() const;
	inline std::shared_ptr<EntityManager> entityManager() const;
	inline std::shared_ptr<CameraManager> cameraManager() const;
	inline std::shared_ptr<EmissionManager> emissionManager() const;
	inline std::shared_ptr<InfiniteLightManager> infiniteLightManager() const;
	inline std::shared_ptr<IntegratorManager> integratorManager() const;
	inline std::shared_ptr<FilterManager> filterManager() const;
	inline std::shared_ptr<SamplerManager> samplerManager() const;
	inline std::shared_ptr<ResourceManager> resourceManager() const;
	inline std::shared_ptr<Cache> cache() const;

	inline ParametricBlob getSpectrum(const std::string& name) const;
	inline bool hasSpectrum(const std::string& name) const;
	inline void addSpectrum(const std::string& name, const ParametricBlob& spec);

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

	inline void addShadingSocket(const std::string& name,
								 const ShadingSocketVariantPtr& output);
	template <typename Socket>
	inline std::shared_ptr<Socket> getShadingSocket(const std::string& name) const;
	inline bool hasShadingSocket(const std::string& name) const;
	template <typename Socket>
	inline bool isShadingSocket(const std::string& name) const;

	inline std::shared_ptr<FloatSpectralMapSocket> getMapSocket(const std::string& name) const;
	inline bool hasMapSocket(const std::string& name) const;
	inline void addMapSocket(const std::string& name, const std::shared_ptr<FloatSpectralMapSocket>& m);

	// Lookup functions for easier access
	std::shared_ptr<FloatSpectralShadingSocket> lookupSpectralShadingSocket(
		const Parameter& parameter, float def = 1) const;
	std::shared_ptr<FloatSpectralShadingSocket> lookupSpectralShadingSocket(
		const Parameter& parameter, const ParametricBlob& def) const;
	std::shared_ptr<FloatScalarShadingSocket> lookupScalarShadingSocket(
		const Parameter& parameter, float def = 1) const;

	std::shared_ptr<FloatSpectralMapSocket> lookupSpectralMapSocket(
		const Parameter& parameter,
		float def = 1) const;
	std::shared_ptr<FloatSpectralMapSocket> lookupSpectralMapSocket(
		const Parameter& parameter,
		const ParametricBlob& def) const;

	inline void* textureSystem();

	inline void setWorkingDir(const std::wstring& dir);
	inline std::wstring workingDir() const;

	inline OutputSpecification& outputSpecification();
	inline const RenderSettings& renderSettings() const;
	inline RenderSettings& renderSettings();

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, const OutputSaveOptions& options = OutputSaveOptions()) const;

	std::shared_ptr<IIntegrator> createSelectedIntegrator() const;
	std::shared_ptr<RenderFactory> createRenderFactory();

	inline std::shared_ptr<SpectralUpsampler> defaultSpectralUpsampler() const { return mDefaultSpectralUpsampler; }

private:
	void loadPlugins(const std::wstring& basedir);

	std::wstring mWorkingDir;
	RenderSettings mRenderSettings;

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
	std::shared_ptr<ResourceManager> mResourceManager;

	std::shared_ptr<SpectralUpsampler> mDefaultSpectralUpsampler;

	std::shared_ptr<Cache> mCache;

	std::map<std::string, ParametricBlob> mSpectrums;
	std::map<std::string, std::shared_ptr<IEmission>> mEmissions;
	std::map<std::string, std::shared_ptr<IMaterial>> mMaterials;
	std::map<std::string, std::shared_ptr<MeshBase>> mMeshes;
	std::map<std::string, ShadingSocketVariantPtr> mNamedShadingSockets;
	std::map<std::string, std::shared_ptr<FloatSpectralMapSocket>> mNamedMapSockets;

	void* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR

#include "Environment.inl"