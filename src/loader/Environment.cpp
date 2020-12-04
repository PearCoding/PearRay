#include "Environment.h"
#include "Platform.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "ServiceObserver.h"
#include "cache/Cache.h"
#include "camera/CameraManager.h"
#include "camera/ICamera.h"
#include "emission/EmissionManager.h"
#include "emission/IEmission.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "filter/FilterManager.h"
#include "filter/IFilterPlugin.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/InfiniteLightManager.h"
#include "integrator/IntegratorManager.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "parameter/Parameter.h"
#include "plugin/PluginManager.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderSettings.h"
#include "sampler/ISamplerPlugin.h"
#include "sampler/SamplerManager.h"
#include "scene/Scene.h"
#include "scene/SceneDatabase.h"
#include "shader/ConstNode.h"
#include "shader/NodeManager.h"
#include "spectral/ISpectralMapperPlugin.h"
#include "spectral/SpectralMapperManager.h"

#include "DefaultSRGB.h"

#include "Logger.h"

#include <OpenImageIO/texture.h>

#include <filesystem>
#include <regex>
#include <typeinfo>

namespace PR {
Environment::Environment(const std::filesystem::path& workdir,
						 const std::filesystem::path& plugdir,
						 bool useStandardLib)
	: mWorkingDir(workdir)
	, mQueryMode(workdir.empty())
	, mServiceObserver(std::make_shared<ServiceObserver>())
	, mSceneDatabase(std::make_shared<SceneDatabase>())
	, mPluginManager(std::make_shared<PluginManager>(plugdir))
	, mCameraManager(std::make_shared<CameraManager>())
	, mEmissionManager(std::make_shared<EmissionManager>())
	, mEntityManager(std::make_shared<EntityManager>())
	, mFilterManager(std::make_shared<FilterManager>())
	, mInfiniteLightManager(std::make_shared<InfiniteLightManager>())
	, mIntegratorManager(std::make_shared<IntegratorManager>())
	, mMaterialManager(std::make_shared<MaterialManager>())
	, mNodeManager(std::make_shared<NodeManager>())
	, mSamplerManager(std::make_shared<SamplerManager>())
	, mSpectralMapperManager(std::make_shared<SpectralMapperManager>())
	, mDefaultSpectralUpsampler(DefaultSRGB::loadSpectralUpsampler())
	, mResourceManager(std::make_shared<ResourceManager>(workdir))
	, mCache(std::make_shared<Cache>(workdir))
	, mTextureSystem(nullptr)
	, mOutputSpecification(workdir)
{
	try {
		loadPlugins(plugdir);
	} catch (const std::exception& e) {
		PR_LOG(L_ERROR) << "Error while loading plugins: " << e.what() << std::endl;
	}

	auto ts = OIIO::TextureSystem::create();
	ts->attribute("automip", 0);
	ts->attribute("gray_to_rgb", 1);
	ts->attribute("accept_untiled", 1);
	ts->attribute("accept_unmipped", 1);
	ts->attribute("forcefloat", 1);
	
	mTextureSystem = ts;

	if (useStandardLib) {
		//Defaults
		auto addColor = [&](const std::string& name, float r, float g, float b) {
			ParametricBlob params;
			mDefaultSpectralUpsampler->prepare(&r, &g, &b, &params[0], &params[1], &params[2], 1);
			mSceneDatabase->Nodes->add(name, std::make_shared<ParametricSpectralNode>(params));
		};

		addColor("black", 0, 0, 0);
		addColor("white", 1, 1, 1);
		addColor("red", 1, 0, 0);
		addColor("green", 0, 1, 0);
		addColor("blue", 0, 0, 1);
		addColor("magenta", 1, 0, 1);
		addColor("yellow", 1, 1, 0);
		addColor("cyan", 0, 1, 1);
		addColor("gray", 0.5f, 0.5f, 0.5f);
		addColor("lightGray", 0.666f, 0.666f, 0.666f);
		addColor("darkGray", 0.333f, 0.333f, 0.333f);
	}
}

Environment::~Environment()
{
	OIIO::TextureSystem::destroy((OIIO::TextureSystem*)mTextureSystem);
}

void Environment::dumpInformation() const
{
	for (auto p : mSceneDatabase->Entities->getAll())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;

	for (auto p : mSceneDatabase->Emissions->getAllNamed())
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << mSceneDatabase->Emissions->get(p.second)->dumpInformation() << std::endl;

	for (auto p : mSceneDatabase->InfiniteLights->getAll())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;

	for (auto p : mSceneDatabase->Materials->getAllNamed())
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << mSceneDatabase->Materials->get(p.second)->dumpInformation() << std::endl;

	for (auto p : mSceneDatabase->Nodes->getAllNamed())
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << mSceneDatabase->Nodes->get(p.second)->dumpInformation() << std::endl;
}

void Environment::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_LOG(L_INFO) << "Initializing output" << std::endl;
	mOutputSpecification.setup(renderer);
}

void Environment::save(RenderContext* renderer, ToneMapper& toneMapper, const OutputSaveOptions& options) const
{
	mOutputSpecification.save(renderer, toneMapper, options);
}

std::shared_ptr<IIntegrator> Environment::createSelectedIntegrator() const
{
	const auto integrator = mRenderSettings.createIntegrator();
	if (!integrator) {
		PR_LOG(L_ERROR) << "Integrator not found!" << std::endl;
		return nullptr;
	}
	return integrator;
}

std::shared_ptr<RenderFactory> Environment::createRenderFactory()
{
	setupFloatingPointEnvironment();

	std::shared_ptr<ICamera> activeCamera = mCameraManager->getActiveCamera();
	if (!activeCamera) {
		PR_LOG(L_ERROR) << "No camera available!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<Scene> scene;
	try {
		scene = std::make_shared<Scene>(mServiceObserver,
										activeCamera,
										mSceneDatabase);
	} catch (const std::exception& e) {
		PR_LOG(L_ERROR) << e.what() << std::endl;
		return nullptr;
	}

	if (!scene) {
		PR_LOG(L_ERROR) << "Could not create scene!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<RenderFactory> fct = std::make_shared<RenderFactory>(scene);

	if (!mSamplerManager->createDefaultsIfNecessary(this))
		return nullptr;

	if (!mFilterManager->createDefaultsIfNecessary(this))
		return nullptr;

	if (!mSpectralMapperManager->createDefaultsIfNecessary(this))
		return nullptr;

	fct->settings() = mRenderSettings;
	return fct;
}

void Environment::loadPlugins(const std::filesystem::path& basedir)
{
	// First load possible embedded plugins
	mPluginManager->loadEmbeddedPlugins();

	try {
#ifdef PR_DEBUG
		static const std::wregex e(L"(lib)?pr_pl_([\\w_]+)_d");
#else
		static const std::wregex e(L"(lib)?pr_pl_([\\w_]+)");
#endif

		// Load dlls
		for (auto& entry : std::filesystem::directory_iterator(basedir)) {
			if (!std::filesystem::is_regular_file(entry))
				continue;

			const std::wstring filename = entry.path().stem().generic_wstring();
			const std::wstring ext		= entry.path().extension().generic_wstring();

			if (ext != L".so" && ext != L".dll")
				continue;

			std::wsmatch what;
			if (std::regex_match(filename, what, e)) {
#ifndef PR_DEBUG
				// Ignore debug builds
				if (filename.substr(filename.size() - 2, 2) == L"_d")
					continue;
#endif

				mPluginManager->load(entry.path());
			}
		}
	} catch (const std::exception& e) {
		PR_LOG(L_ERROR) << "Could not load external plugins: " << e.what() << std::endl;
	}

	// Load into respective managers
	for (auto plugin : mPluginManager->plugins()) {
		const char* type = plugin->type();
		if (type == nullptr) {
			PR_LOG(L_ERROR) << "Plugin " << typeid(plugin).name() << " returns null type" << std::endl;
			continue;
		}

		std::string stype = type;
		std::transform(stype.begin(), stype.end(), stype.begin(), ::tolower);

		if (stype == "integrator")
			mIntegratorManager->addFactory(std::dynamic_pointer_cast<IIntegratorPlugin>(plugin));
		else if (stype == "camera")
			mCameraManager->addFactory(std::dynamic_pointer_cast<ICameraPlugin>(plugin));
		else if (stype == "material")
			mMaterialManager->addFactory(std::dynamic_pointer_cast<IMaterialPlugin>(plugin));
		else if (stype == "emission")
			mEmissionManager->addFactory(std::dynamic_pointer_cast<IEmissionPlugin>(plugin));
		else if (stype == "infinitelight")
			mInfiniteLightManager->addFactory(std::dynamic_pointer_cast<IInfiniteLightPlugin>(plugin));
		else if (stype == "entity")
			mEntityManager->addFactory(std::dynamic_pointer_cast<IEntityPlugin>(plugin));
		else if (stype == "filter")
			mFilterManager->addFactory(std::dynamic_pointer_cast<IFilterPlugin>(plugin));
		else if (stype == "sampler")
			mSamplerManager->addFactory(std::dynamic_pointer_cast<ISamplerPlugin>(plugin));
		else if (stype == "node")
			mNodeManager->addFactory(std::dynamic_pointer_cast<INodePlugin>(plugin));
		else if (stype == "spectralmapper")
			mSpectralMapperManager->addFactory(std::dynamic_pointer_cast<ISpectralMapperPlugin>(plugin));
		else
			PR_LOG(L_ERROR) << "Plugin " << typeid(plugin).name() << " has unknown type '" << type << "'" << std::endl;
	}
}

} // namespace PR
