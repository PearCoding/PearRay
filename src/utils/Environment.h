#pragma once

#include "output/OutputSpecification.h"
#include "registry/Registry.h"
#include "shader/Socket.h"
#include "spectral/Spectrum.h"

#include <boost/variant.hpp>

#include <list>
#include <map>
#include <utility>

namespace PR {
class IEmission;
class IMaterial;
class MeshContainer;
class SpectrumDescriptor;
class PluginManager;
class MaterialManager;
class EmissionManager;
class EntityManager;
class CameraManager;
class InfiniteLightManager;
class IntegratorManager;
class IIntegrator;
class RenderFactory;

using ShadingSocketVariantPtr = boost::variant<
	std::shared_ptr<FloatScalarShadingSocket>,
	std::shared_ptr<FloatSpectralShadingSocket>,
	std::shared_ptr<FloatVectorShadingSocket>>;

class BadRenderEnvironment : public std::exception {
public:
	const char* what() const throw()
	{
		return "Bad Render Environment";
	}
};

class PR_LIB_UTILS Environment {
public:
	explicit Environment(const std::wstring& workdir,
						 const std::shared_ptr<SpectrumDescriptor>& specDesc,
						 const std::wstring& plugdir,
						 bool useStandardLib = true);
	virtual ~Environment();

	inline std::shared_ptr<SpectrumDescriptor> spectrumDescriptor() const;

	inline std::shared_ptr<PluginManager> pluginManager() const;
	inline std::shared_ptr<MaterialManager> materialManager() const;
	inline std::shared_ptr<EntityManager> entityManager() const;
	inline std::shared_ptr<CameraManager> cameraManager() const;
	inline std::shared_ptr<EmissionManager> emissionManager() const;
	inline std::shared_ptr<InfiniteLightManager> infiniteLightManager() const;
	inline std::shared_ptr<IntegratorManager> integratorManager() const;

	inline Spectrum getSpectrum(const std::string& name) const;
	inline bool hasSpectrum(const std::string& name) const;
	inline void addSpectrum(const std::string& name, const Spectrum& spec);

	inline std::shared_ptr<IEmission> getEmission(const std::string& name) const;
	inline bool hasEmission(const std::string& name) const;
	inline void addEmission(const std::string& name, const std::shared_ptr<IEmission>& mat);
	inline size_t emissionCount() const;

	inline std::shared_ptr<IMaterial> getMaterial(const std::string& name) const;
	inline bool hasMaterial(const std::string& name) const;
	inline void addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat);
	inline size_t materialCount() const;

	inline std::shared_ptr<MeshContainer> getMesh(const std::string& name) const;
	inline bool hasMesh(const std::string& name) const;
	inline void addMesh(const std::string& name, const std::shared_ptr<MeshContainer>& m);

	inline void addShadingSocket(const std::string& name,
								 const ShadingSocketVariantPtr& output);
	template <typename Socket>
	inline std::shared_ptr<Socket> getShadingSocket(const std::string& name) const;
	inline bool hasShadingSocket(const std::string& name) const;
	template <typename Socket>
	inline bool isShadingSocket(const std::string& name) const;

	std::shared_ptr<FloatSpectralShadingSocket> getSpectralShadingSocket(
		const std::string& name, float def = 1) const;
	std::shared_ptr<FloatSpectralShadingSocket> getSpectralShadingSocket(
		const std::string& name, const Spectrum& def) const;
	std::shared_ptr<FloatScalarShadingSocket> getScalarShadingSocket(
		const std::string& name, float def = 1) const;

	inline std::shared_ptr<FloatSpectralMapSocket> getMapSocket(const std::string& name) const;
	inline bool hasMapSocket(const std::string& name) const;
	inline void addMapSocket(const std::string& name,
							 const std::shared_ptr<FloatSpectralMapSocket>& m);

	std::shared_ptr<FloatSpectralMapSocket> getSpectralMapSocket(
		const std::string& name,
		float def = 1) const;
	std::shared_ptr<FloatSpectralMapSocket> getSpectralMapSocket(
		const std::string& name,
		const Spectrum& def) const;

	inline void* textureSystem();

	inline void setWorkingDir(const std::wstring& dir);
	inline std::wstring workingDir() const;

	inline OutputSpecification& outputSpecification();
	inline const Registry& registry() const;
	inline Registry& registry();

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force = false) const;

	std::shared_ptr<IIntegrator> createSelectedIntegrator() const;
	std::shared_ptr<RenderFactory> createRenderFactory() const;

private:
	std::shared_ptr<FloatSpectralShadingSocket> lookupSpectralShadingSocket(
		const std::string& name) const;
	std::shared_ptr<FloatScalarShadingSocket> lookupScalarShadingSocket(
		const std::string& name) const;

	void loadPlugins(const std::wstring& basedir);
	void loadOnePlugin(const std::wstring& name);

	Registry mRegistry;

	std::wstring mWorkingDir;
	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;

	// Order matters: PluginManager should be before other managers
	std::shared_ptr<PluginManager> mPluginManager;
	std::shared_ptr<MaterialManager> mMaterialManager;
	std::shared_ptr<EntityManager> mEntityManager;
	std::shared_ptr<CameraManager> mCameraManager;
	std::shared_ptr<EmissionManager> mEmissionManager;
	std::shared_ptr<InfiniteLightManager> mInfiniteLightManager;
	std::shared_ptr<IntegratorManager> mIntegratorManager;

	std::map<std::string, PR::Spectrum> mSpectrums;
	std::map<std::string, std::shared_ptr<IEmission>> mEmissions;
	std::map<std::string, std::shared_ptr<IMaterial>> mMaterials;
	std::map<std::string, std::shared_ptr<MeshContainer>> mMeshes;
	std::map<std::string, ShadingSocketVariantPtr> mNamedShadingSockets;
	std::map<std::string, std::shared_ptr<FloatSpectralMapSocket>> mNamedMapSockets;

	void* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR

#include "Environment.inl"