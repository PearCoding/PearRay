#include "QueryEnvironment.h"
#include "Platform.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
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
#include "shader/ConstNode.h"
#include "shader/NodeManager.h"
#include "spectral/ISpectralMapperPlugin.h"
#include "spectral/SpectralMapperManager.h"

#include "DefaultSRGB.h"

#include "Logger.h"

#include <OpenImageIO/texture.h>

#include <filesystem>
#include <regex>

namespace PR {
QueryEnvironment::QueryEnvironment(const std::filesystem::path& plugdir)
	: mPluginManager(std::make_shared<PluginManager>(plugdir))
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
{
	mDefaultSpectralUpsampler = DefaultSRGB::loadSpectralUpsampler();

	try {
		loadPlugins(plugdir);
	} catch (const std::exception& e) {
		PR_LOG(L_ERROR) << "Error while loading plugins: " << e.what() << std::endl;
	}
}

QueryEnvironment::~QueryEnvironment()
{
}

void QueryEnvironment::loadPlugins(const std::filesystem::path& basedir)
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
		switch (plugin->type()) {
		case PT_INTEGRATOR:
			mIntegratorManager->addFactory(std::dynamic_pointer_cast<IIntegratorPlugin>(plugin));
			break;
		case PT_CAMERA:
			mCameraManager->addFactory(std::dynamic_pointer_cast<ICameraPlugin>(plugin));
			break;
		case PT_MATERIAL:
			mMaterialManager->addFactory(std::dynamic_pointer_cast<IMaterialPlugin>(plugin));
			break;
		case PT_EMISSION:
			mEmissionManager->addFactory(std::dynamic_pointer_cast<IEmissionPlugin>(plugin));
			break;
		case PT_INFINITELIGHT:
			mInfiniteLightManager->addFactory(std::dynamic_pointer_cast<IInfiniteLightPlugin>(plugin));
			break;
		case PT_ENTITY:
			mEntityManager->addFactory(std::dynamic_pointer_cast<IEntityPlugin>(plugin));
			break;
		case PT_FILTER:
			mFilterManager->addFactory(std::dynamic_pointer_cast<IFilterPlugin>(plugin));
			break;
		case PT_SAMPLER:
			mSamplerManager->addFactory(std::dynamic_pointer_cast<ISamplerPlugin>(plugin));
			break;
		case PT_NODE:
			mNodeManager->addFactory(std::dynamic_pointer_cast<INodePlugin>(plugin));
			break;
		case PT_SPECTRALMAPPER:
			mSpectralMapperManager->addFactory(std::dynamic_pointer_cast<ISpectralMapperPlugin>(plugin));
			break;
		default:
			break;
		}
	}
}

} // namespace PR
