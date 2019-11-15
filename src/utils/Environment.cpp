#include "Environment.h"

#include "camera/CameraManager.h"
#include "camera/ICamera.h"
#include "emission/EmissionManager.h"
#include "emission/IEmission.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/InfiniteLightManager.h"
#include "integrator/IntegratorManager.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "mesh/TriMesh.h"
#include "plugin/PluginManager.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderSettings.h"
#include "scene/Scene.h"
#include "shader/ConstSocket.h"
#include "shader/MapShadingSocket.h"
#include "spectral/RGBConverter.h"
#include "spectral/SpectrumDescriptor.h"
#include "spectral/XYZConverter.h"

#include "Logger.h"

#include <OpenImageIO/texture.h>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>

namespace PR {
Environment::Environment(const std::wstring& workdir,
						 const std::shared_ptr<SpectrumDescriptor>& specDesc,
						 const std::wstring& plugdir,
						 bool useStandardLib)
	: mWorkingDir(workdir)
	, mSpectrumDescriptor(specDesc)
	, mPluginManager(std::make_shared<PluginManager>())
	, mMaterialManager(std::make_shared<MaterialManager>())
	, mEntityManager(std::make_shared<EntityManager>())
	, mCameraManager(std::make_shared<CameraManager>())
	, mEmissionManager(std::make_shared<EmissionManager>())
	, mInfiniteLightManager(std::make_shared<InfiniteLightManager>())
	, mIntegratorManager(std::make_shared<IntegratorManager>())
	, mTextureSystem(nullptr)
	, mOutputSpecification(workdir)
{
	registry().setByGroup(RG_RENDERER, "plugins/path", plugdir);
	mTextureSystem = OIIO::TextureSystem::create();

	if (useStandardLib) {
		//Defaults
		auto addColor = [&](const std::string& name, float r, float g, float b) {
			Spectrum c(specDesc);
			RGBConverter::toSpec(c, r, g, b);
			mSpectrums.insert(std::make_pair(name, c));
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

	loadPlugins(plugdir);
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
}

void Environment::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_LOG(L_INFO) << "Initializing output" << std::endl;
	mOutputSpecification.setup(renderer);
	freeze(renderer);
}

void Environment::save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force) const
{
	mOutputSpecification.save(renderer, toneMapper, force);
}

void Environment::loadPlugins(const std::wstring& basedir)
{
#ifdef PR_DEBUG
	static const boost::wregex e(L"(lib)?pr_pl_([\\w_]+)_d");
#else
	static const boost::wregex e(L"(lib)?pr_pl_([\\w_]+)");
#endif

	for (auto& entry :
		 boost::make_iterator_range(boost::filesystem::directory_iterator(basedir), {})) {
		if (!boost::filesystem::is_regular_file(entry))
			continue;

		const std::wstring filename = entry.path().stem().generic_wstring();
		const std::wstring ext		= entry.path().extension().generic_wstring();

		if (ext != L".so" && ext != L".dll")
			continue;

		boost::wsmatch what;
		if (boost::regex_match(filename, what, e)) {
#ifndef PR_DEBUG
			// Ignore debug builds
			if (filename.substr(filename.size() - 2, 2) == L"_d")
				continue;
#endif

			loadOnePlugin(entry.path().generic_wstring());
		}
	}
}

void Environment::loadOnePlugin(const std::wstring& name)
{
	auto plugin = mPluginManager->load(name, mRegistry);
	if (!plugin)
		return;

	switch (plugin->type()) {
	case PT_INTEGRATOR:
		mIntegratorManager->addFactory(std::dynamic_pointer_cast<IIntegratorFactory>(plugin));
		break;
	case PT_CAMERA:
		mCameraManager->addFactory(std::dynamic_pointer_cast<ICameraFactory>(plugin));
		break;
	case PT_MATERIAL:
		mMaterialManager->addFactory(std::dynamic_pointer_cast<IMaterialFactory>(plugin));
		break;
	case PT_EMISSION:
		mEmissionManager->addFactory(std::dynamic_pointer_cast<IEmissionFactory>(plugin));
		break;
	case PT_INFINITELIGHT:
		mInfiniteLightManager->addFactory(std::dynamic_pointer_cast<IInfiniteLightFactory>(plugin));
		break;
	case PT_ENTITY:
		mEntityManager->addFactory(std::dynamic_pointer_cast<IEntityFactory>(plugin));
		break;
	default:
		PR_LOG(L_ERROR) << "Plugin " << boost::filesystem::path(name) << " has unknown plugin type." << std::endl;
		return;
	}
}

std::shared_ptr<IIntegrator> Environment::createSelectedIntegrator() const
{
	const std::string intMode = mRegistry.getByGroup<std::string>(RG_RENDERER,
																  "common/type",
																  "direct");
	auto intfact			  = mIntegratorManager->getFactory(intMode);
	if (!intfact) {
		PR_LOG(L_ERROR) << "Integrator " << intMode
						<< " not found!" << std::endl;
		return nullptr;
	}

	auto integrator = intfact->create(0, 0, *this);
	if (!integrator) {
		PR_LOG(L_ERROR) << "Integrator " << intMode
						<< " implementation is broken! Please contact plugin developer." << std::endl;
		return nullptr;
	}
	mIntegratorManager->addObject(integrator);

	return integrator;
}

std::shared_ptr<RenderFactory> Environment::createRenderFactory() const
{
	auto entities = mEntityManager->getAll();
	if (entities.empty()) {
		PR_LOG(L_ERROR) << "No entities available!" << std::endl;
		return nullptr;
	}

	auto materials = mMaterialManager->getAll();
	if (materials.empty()) {
		PR_LOG(L_ERROR) << "No materials available!" << std::endl;
		return nullptr;
	}

	auto emissions = mEmissionManager->getAll();
	auto inflights = mInfiniteLightManager->getAll();

	std::shared_ptr<ICamera> activeCamera = mCameraManager->getActiveCamera();
	if (!activeCamera) {
		PR_LOG(L_ERROR) << "No camera available!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<Scene> scene = std::make_shared<Scene>(activeCamera,
														   entities,
														   materials,
														   emissions,
														   inflights,
														   mWorkingDir + L"/scene.cnt");
	if (!scene) {
		PR_LOG(L_ERROR) << "Could not create scene!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<RenderFactory> fct = std::make_shared<RenderFactory>(scene,
																		 mSpectrumDescriptor);

	// Populate settings from the registry
	fct->settings().seed = mRegistry.getByGroup<uint64>(
		RG_RENDERER,
		"common/seed",
		42);
	fct->settings().maxRayDepth = std::max<uint32>(
		1,
		mRegistry.getByGroup<uint32>(
			RG_RENDERER,
			"common/max_ray_depth",
			1));
	fct->settings().aaSampleCount = std::max<uint64>(
		1,
		mRegistry.getByGroup<uint64>(
			RG_RENDERER,
			"common/sampler/aa/count",
			1));
	fct->settings().lensSampleCount = std::max<uint64>(
		1,
		mRegistry.getByGroup<uint64>(
			RG_RENDERER,
			"common/sampler/lens/count",
			1));
	fct->settings().timeSampleCount = std::max<uint64>(
		1,
		mRegistry.getByGroup<uint64>(
			RG_RENDERER,
			"common/sampler/time/count",
			1));
	fct->settings().aaSampler = mRegistry.getByGroup<SamplerMode>(
		RG_RENDERER,
		"common/sampler/aa/type",
		SM_MULTI_JITTER);
	fct->settings().lensSampler = mRegistry.getByGroup<SamplerMode>(
		RG_RENDERER,
		"common/sampler/lens/type",
		SM_MULTI_JITTER);
	fct->settings().timeSampler = mRegistry.getByGroup<SamplerMode>(
		RG_RENDERER,
		"common/sampler/time/type",
		SM_MULTI_JITTER);
	fct->settings().timeMappingMode = mRegistry.getByGroup<TimeMappingMode>(
		RG_RENDERER,
		"common/sampler/time/mapping",
		TMM_RIGHT);
	fct->settings().timeScale = mRegistry.getByGroup<float>(
		RG_RENDERER,
		"common/sampler/time/scale",
		1.0);
	fct->settings().tileMode = mRegistry.getByGroup<TileMode>(
		RG_RENDERER,
		"common/tile/mode",
		TM_LINEAR);
	fct->settings().filmWidth = std::max<uint32>(
		1,
		mRegistry.getByGroup<uint32>(
			RG_RENDERER,
			"film/width",
			1920));
	fct->settings().filmHeight = std::max<uint32>(
		1,
		mRegistry.getByGroup<uint32>(
			RG_RENDERER,
			"film/height",
			1080));
	fct->settings().cropMinX = std::max<float>(
		0,
		std::min<float>(1,
						(float)mRegistry.getByGroup<uint32>(
							RG_RENDERER,
							"film/crop/min_x",
							0)));
	fct->settings().cropMaxX = std::max<float>(
		0,
		std::min<float>(1,
						(float)mRegistry.getByGroup<uint32>(
							RG_RENDERER,
							"film/crop/max_x",
							1)));
	fct->settings().cropMinY = std::max<float>(
		0,
		std::min<float>(1,
						(float)mRegistry.getByGroup<uint32>(
							RG_RENDERER,
							"film/crop/min_y",
							0)));
	fct->settings().cropMaxY = std::max<float>(
		0,
		std::min<float>(1,
						(float)mRegistry.getByGroup<uint32>(
							RG_RENDERER,
							"film/crop/max_y",
							1)));
	return fct;
}

void Environment::freeze(const std::shared_ptr<RenderContext>& ctx)
{
	PR_LOG(L_INFO) << "Freezing scene" << std::endl;
	for (auto e : mEmissionManager->getAll())
		e->freeze(ctx.get());

	for (auto e : mMaterialManager->getAll())
		e->freeze(ctx.get());

	for (auto e : mInfiniteLightManager->getAll())
		e->freeze(ctx.get());

	for (auto e : mEntityManager->getAll())
		e->freeze(ctx.get());

	for (auto e : mCameraManager->getAll())
		e->freeze(ctx.get());
}

std::shared_ptr<FloatSpectralShadingSocket> Environment::getSpectralShadingSocket(
	const std::string& name, float def) const
{
	auto socket = lookupSpectralShadingSocket(name);
	if (socket)
		return socket;
	else
		return std::make_shared<ConstSpectralShadingSocket>(Spectrum(mSpectrumDescriptor, def));
}

std::shared_ptr<FloatSpectralShadingSocket> Environment::getSpectralShadingSocket(
	const std::string& name, const Spectrum& def) const
{
	auto socket = lookupSpectralShadingSocket(name);
	if (socket)
		return socket;
	else
		return std::make_shared<ConstSpectralShadingSocket>(def);
}

/* Allows input of:
 FLOAT
 SPECTRUM_NAME
 SOCKET_NAME
*/
std::shared_ptr<FloatSpectralShadingSocket> Environment::lookupSpectralShadingSocket(
	const std::string& name) const
{
	try {
		float val = std::stof(name);
		return std::make_shared<ConstSpectralShadingSocket>(Spectrum(mSpectrumDescriptor, val));
	} catch (const std::invalid_argument&) {
		// Nothing
	}

	if (name.find_first_of("{t} ") == 0) {
		std::string tname = name.substr(4);

		if (hasShadingSocket(tname)) {
			auto socket = getShadingSocket<FloatSpectralShadingSocket>(tname);
			if (socket)
				return socket;
		} else if(hasMapSocket(tname)) {
			auto socket = getMapSocket(tname);
			if (socket)
				return std::make_shared<MapShadingSocket>(socket);
		}
	}

	if (hasSpectrum(name))
		return std::make_shared<ConstSpectralShadingSocket>(getSpectrum(name));

	return nullptr;
}

std::shared_ptr<FloatScalarShadingSocket> Environment::getScalarShadingSocket(
	const std::string& name, float def) const
{
	auto socket = lookupScalarShadingSocket(name);
	if (socket)
		return socket;
	else
		return std::make_shared<ConstScalarShadingSocket>(def);
}

/* Allows input of:
 FLOAT
 SOCKET_NAME
*/
std::shared_ptr<FloatScalarShadingSocket> Environment::lookupScalarShadingSocket(
	const std::string& name) const
{
	try {
		float val = std::stof(name);
		return std::make_shared<ConstScalarShadingSocket>(val);
	} catch (const std::invalid_argument&) {
		// Nothing
	}

	if (name.find_first_of("{t} ") == 0) {
		std::string tname = name.substr(4);

		if (hasShadingSocket(tname)) {
			auto socket = getShadingSocket<FloatScalarShadingSocket>(tname);
			if (socket)
				return socket;
		}
	}

	return nullptr;
}

///////////////////
std::shared_ptr<FloatSpectralMapSocket> Environment::getSpectralMapSocket(
	const std::string& name,
	float def) const
{
	return getSpectralMapSocket(name, Spectrum(mSpectrumDescriptor, def));
}

std::shared_ptr<FloatSpectralMapSocket> Environment::getSpectralMapSocket(
	const std::string& name,
	const Spectrum& def) const
{
	try {
		float val = std::stof(name);
		return std::make_shared<ConstSpectralMapSocket>(Spectrum(mSpectrumDescriptor, val));
	} catch (const std::invalid_argument&) {
		// Nothing
	}

	if (name.find_first_of("{t} ") == 0) {
		std::string tname = name.substr(4);

		if (hasMapSocket(tname)) {
			auto socket = getMapSocket(tname);
			if (socket)
				return socket;
		}
	}

	if (hasSpectrum(name))
		return std::make_shared<ConstSpectralMapSocket>(getSpectrum(name));

	return std::make_shared<ConstSpectralMapSocket>(def);
}
} // namespace PR
