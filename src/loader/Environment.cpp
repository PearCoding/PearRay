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
Environment::Environment(const std::filesystem::path& workdir,
						 const std::filesystem::path& plugdir,
						 bool useStandardLib)
	: QueryEnvironment(plugdir)
	, mWorkingDir(workdir)
	, mResourceManager(std::make_shared<ResourceManager>(workdir))
	, mCache(std::make_shared<Cache>(workdir))
	, mTextureSystem(nullptr)
	, mOutputSpecification(workdir)
{
	mTextureSystem = OIIO::TextureSystem::create();

	if (useStandardLib) {
		//Defaults
		// TODO: Involve upsampler!
		auto addColor = [&](const std::string& name, float r, float g, float b) {
			ParametricBlob params;
			mDefaultSpectralUpsampler->prepare(&r, &g, &b, &params[0], &params[1], &params[2], 1);
			mNamedNodes.insert(std::make_pair(name, std::make_shared<ParametricSpectralNode>(params)));
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
	for (auto p : mEntityManager->getAll())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;

	/*for (auto p : mEmissionManager->getAll())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;*/

	for (auto p : mInfiniteLightManager->getAll())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;

	for (auto p : mMaterials)
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << p.second->dumpInformation() << std::endl;

	for (auto p : mNamedNodes)
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << p.second->dumpInformation() << std::endl;
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

	const auto& entities  = mEntityManager->getAll();
	const auto& materials = mMaterialManager->getAll();
	const auto& emissions = mEmissionManager->getAll();
	const auto& inflights = mInfiniteLightManager->getAll();
	const auto& nodes	  = mNodeManager->getAll();

	std::shared_ptr<ICamera> activeCamera = mCameraManager->getActiveCamera();
	if (!activeCamera) {
		PR_LOG(L_ERROR) << "No camera available!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<Scene> scene;
	try {
		scene = std::make_shared<Scene>(mServiceObserver,
										activeCamera,
										entities,
										materials,
										emissions,
										inflights,
										nodes);
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

std::shared_ptr<INode> Environment::getRawNode(uint64 refid) const
{
	if (refid != P_INVALID_REFERENCE && mNodeManager->hasObject(refid))
		return mNodeManager->getObject(refid);
	else
		return nullptr;
}

} // namespace PR
