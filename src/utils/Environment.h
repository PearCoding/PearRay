#pragma once

#include "output/OutputSpecification.h"
#include "registry/Registry.h"
#include "shader/ShadingSocket.h"
#include "spectral/Spectrum.h"

#include <OpenImageIO/texture.h>
#include <boost/variant.hpp>

#include <list>
#include <map>
#include <utility>

namespace PR {
class IEmission;
class IMaterial;
class TriMesh;
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
	explicit Environment(const std::string& workdir,
						 const std::shared_ptr<SpectrumDescriptor>& specDesc,
						 const std::string& plugdir,
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

	inline std::shared_ptr<TriMesh> getMesh(const std::string& name) const;
	inline bool hasMesh(const std::string& name) const;
	inline void addMesh(const std::string& name, const std::shared_ptr<TriMesh>& m);

	inline void addShadingSocket(const std::string& name,
								 const ShadingSocketVariantPtr& output);
	template <typename Socket>
	inline std::shared_ptr<Socket> getShadingSocket(const std::string& name) const;
	inline bool hasShadingSocket(const std::string& name) const;
	template <typename Socket>
	inline bool isShadingSocket(const std::string& name) const;

	std::shared_ptr<FloatSpectralShadingSocket> getSpectralShadingSocket(
		const std::string& name, float def = 1) const;

	inline OIIO::TextureSystem* textureSystem();

	inline void setWorkingDir(const std::string& dir);
	inline std::string workingDir() const;

	inline OutputSpecification& outputSpecification();
	inline const Registry& registry() const;
	inline Registry& registry();

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force = false) const;

	std::shared_ptr<IIntegrator> createSelectedIntegrator() const;
	std::shared_ptr<RenderFactory> createRenderFactory() const;

private:
	void freeze(const std::shared_ptr<RenderContext>& ctx);
	void loadPlugins(const std::string& basedir);
	void loadOnePlugin(const std::string& name);

	Registry mRegistry;

	std::string mWorkingDir;
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
	std::map<std::string, std::shared_ptr<TriMesh>> mMeshes;
	std::map<std::string, ShadingSocketVariantPtr> mNamedShadingSocket;

	OIIO::TextureSystem* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR

#include "Environment.inl"