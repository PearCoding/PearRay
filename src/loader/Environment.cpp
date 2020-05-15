#include "Environment.h"
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
#include "mesh/Mesh.h"
#include "parameter/Parameter.h"

#include "plugin/PluginManager.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderSettings.h"
#include "sampler/ISamplerPlugin.h"
#include "sampler/SamplerManager.h"
#include "scene/Scene.h"
#include "shader/ConstSocket.h"
#include "shader/MapShadingSocket.h"

#include "DefaultSRGB.h"

#include "Logger.h"

#include <OpenImageIO/texture.h>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>

namespace PR {
Environment::Environment(const std::wstring& workdir,
						 const std::wstring& plugdir,
						 bool useStandardLib)
	: mWorkingDir(workdir)
	, mPluginManager(std::make_shared<PluginManager>(plugdir))
	, mMaterialManager(std::make_shared<MaterialManager>())
	, mEntityManager(std::make_shared<EntityManager>())
	, mCameraManager(std::make_shared<CameraManager>())
	, mEmissionManager(std::make_shared<EmissionManager>())
	, mInfiniteLightManager(std::make_shared<InfiniteLightManager>())
	, mIntegratorManager(std::make_shared<IntegratorManager>())
	, mFilterManager(std::make_shared<FilterManager>())
	, mSamplerManager(std::make_shared<SamplerManager>())
	, mResourceManager(std::make_shared<ResourceManager>(workdir))
	, mCache(std::make_shared<Cache>(workdir))
	, mTextureSystem(nullptr)
	, mOutputSpecification(workdir)
{
	mTextureSystem = OIIO::TextureSystem::create();
	mDefaultSpectralUpsampler = loadDefaultSpectralUpsampler();

	if (useStandardLib) {
		//Defaults
		// TODO: Involve upsampler!
		auto addColor = [&](const std::string& name, float r, float g, float b) {
			ParametricBlob params;
			mDefaultSpectralUpsampler->prepare(&r, &g, &b, &params[0], &params[1], &params[2], 1);
			mSpectrums.insert(std::make_pair(name, params));
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
}

void Environment::save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force) const
{
	mOutputSpecification.save(renderer, toneMapper, force);
}

void Environment::loadPlugins(const std::wstring& basedir)
{
	// First load possible embedded plugins
	mPluginManager->loadEmbeddedPlugins();

#ifdef PR_DEBUG
	static const boost::wregex e(L"(lib)?pr_pl_([\\w_]+)_d");
#else
	static const boost::wregex e(L"(lib)?pr_pl_([\\w_]+)");
#endif

	// Load dlls
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

			mPluginManager->load(entry.path().generic_wstring());
		}
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
		default:
			break;
		}
	}
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
	auto entities  = mEntityManager->getAll();
	auto materials = mMaterialManager->getAll();
	auto emissions = mEmissionManager->getAll();
	auto inflights = mInfiniteLightManager->getAll();

	std::shared_ptr<ICamera> activeCamera = mCameraManager->getActiveCamera();
	if (!activeCamera) {
		PR_LOG(L_ERROR) << "No camera available!" << std::endl;
		return nullptr;
	}

	std::wstring scene_cnt = mResourceManager->requestFile("scene", "global", ".cnt");

	std::shared_ptr<Scene> scene = std::make_shared<Scene>(activeCamera,
														   entities,
														   materials,
														   emissions,
														   inflights,
														   scene_cnt);
	if (!scene) {
		PR_LOG(L_ERROR) << "Could not create scene!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<RenderFactory> fct = std::make_shared<RenderFactory>(scene);

	setupDefaultSampler();
	if (!mRenderSettings.aaSamplerFactory || !mRenderSettings.lensSamplerFactory || !mRenderSettings.timeSamplerFactory)
		return nullptr;

	setupDefaultFilter();
	if (!mRenderSettings.pixelFilterFactory)
		return nullptr;

	fct->settings() = mRenderSettings;
	return fct;
}

void Environment::setupDefaultSampler()
{
	constexpr uint64 DEF_AA_SC	 = 128;
	constexpr uint64 DEF_LENS_SC = 1;
	constexpr uint64 DEF_TIME_SC = 1;

	if (!mRenderSettings.aaSamplerFactory) {
		PR_LOG(L_WARNING) << "No AA sampler selected. Using sobol sampler" << std::endl;
		auto plugin = mSamplerManager->getFactory("sobol");
		if (!plugin) {
			PR_LOG(L_ERROR) << "No sobol sampler found" << std::endl;
			return;
		}

		ParameterGroup params;
		params.addParameter("sample_count", Parameter::fromUInt(DEF_AA_SC));

		SceneLoadContext ctx;
		ctx.Env		   = this;
		ctx.Parameters = params;

		mRenderSettings.aaSamplerFactory = plugin->create(mSamplerManager->nextID(), ctx);
		mSamplerManager->addObject(mRenderSettings.aaSamplerFactory);
		if (!mRenderSettings.aaSamplerFactory) {
			PR_LOG(L_ERROR) << "Could not create sobol sampler" << std::endl;
			return;
		}
	}

	if (!mRenderSettings.lensSamplerFactory) {
		PR_LOG(L_WARNING) << "No lens sampler selected. Using sobol sampler" << std::endl;
		auto plugin = mSamplerManager->getFactory("sobol");
		if (!plugin) {
			PR_LOG(L_ERROR) << "No sobol sampler found" << std::endl;
			return;
		}

		ParameterGroup params;
		params.addParameter("sample_count", Parameter::fromUInt(DEF_TIME_SC));

		SceneLoadContext ctx;
		ctx.Env		   = this;
		ctx.Parameters = params;

		mRenderSettings.lensSamplerFactory = plugin->create(mSamplerManager->nextID(), ctx);
		mSamplerManager->addObject(mRenderSettings.lensSamplerFactory);
		if (!mRenderSettings.lensSamplerFactory) {
			PR_LOG(L_ERROR) << "Could not create sobol sampler" << std::endl;
			return;
		}
	}

	if (!mRenderSettings.timeSamplerFactory) {
		PR_LOG(L_WARNING) << "No time sampler selected. Using sobol sampler" << std::endl;
		auto plugin = mSamplerManager->getFactory("sobol");
		if (!plugin) {
			PR_LOG(L_ERROR) << "No sobol sampler found" << std::endl;
			return;
		}

		ParameterGroup params;
		params.addParameter("sample_count", Parameter::fromUInt(DEF_LENS_SC));

		SceneLoadContext ctx;
		ctx.Env		   = this;
		ctx.Parameters = params;

		mRenderSettings.timeSamplerFactory = plugin->create(mSamplerManager->nextID(), ctx);
		mSamplerManager->addObject(mRenderSettings.timeSamplerFactory);
		if (!mRenderSettings.timeSamplerFactory) {
			PR_LOG(L_ERROR) << "Could not create sobol sampler" << std::endl;
			return;
		}
	}
}

void Environment::setupDefaultFilter()
{
	constexpr int DEF_R = 1;
	if (!mRenderSettings.pixelFilterFactory) {
		PR_LOG(L_WARNING) << "No pixel filter selected. Creating mitchell filter" << std::endl;
		auto plugin = mFilterManager->getFactory("mitchell");
		if (!plugin) {
			PR_LOG(L_ERROR) << "No mitchell filter found" << std::endl;
			return;
		}

		ParameterGroup params;
		params.addParameter("radius", Parameter::fromInt(DEF_R));

		SceneLoadContext ctx;
		ctx.Env		   = this;
		ctx.Parameters = params;

		mRenderSettings.pixelFilterFactory = plugin->create(mFilterManager->nextID(), ctx);
		mFilterManager->addObject(mRenderSettings.pixelFilterFactory);
		if (!mRenderSettings.pixelFilterFactory) {
			PR_LOG(L_ERROR) << "Could not create mitchell filter" << std::endl;
			return;
		}
	}
}

/* Allows input of:
 FLOAT
 SPECTRUM_NAME
 SOCKET_NAME
*/
std::shared_ptr<FloatSpectralShadingSocket> Environment::lookupSpectralShadingSocket(
	const Parameter& parameter, float def) const
{
	return lookupSpectralShadingSocket(parameter, ParametricBlob(def));
}

std::shared_ptr<FloatSpectralShadingSocket> Environment::lookupSpectralShadingSocket(
	const Parameter& parameter, const ParametricBlob& def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstSpectralShadingSocket>(def);
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray())
			return std::make_shared<ConstSpectralShadingSocket>(def);
		else
			return std::make_shared<ConstSpectralShadingSocket>(ParametricBlob(parameter.getNumber(0.0f)));
	case PT_String:
		if (parameter.flags() & PF_Texture) {
			std::string tname = parameter.getString("");

			if (hasMapSocket(tname)) {
				auto socket = getMapSocket(tname);
				if (socket)
					return std::make_shared<MapShadingSocket>(socket);
			}
		} else {
			std::string sname = parameter.getString("");

			if (hasSpectrum(sname))
				return std::make_shared<ConstSpectralShadingSocket>(getSpectrum(sname));
		}
		return std::make_shared<ConstSpectralShadingSocket>(def);
	}
}

/* Allows input of:
 FLOAT
 SOCKET_NAME
*/
std::shared_ptr<FloatScalarShadingSocket> Environment::lookupScalarShadingSocket(
	const Parameter& parameter, float def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstScalarShadingSocket>(def);
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray())
			return std::make_shared<ConstScalarShadingSocket>(def);
		else
			return std::make_shared<ConstScalarShadingSocket>(parameter.getNumber(def));
	case PT_String:
		if (parameter.flags() & PF_Node) {
			std::string tname = parameter.getString("");

			if (hasShadingSocket(tname)) {
				auto socket = getShadingSocket<FloatScalarShadingSocket>(tname);
				if (socket)
					return socket;
			}
		}
		return std::make_shared<ConstScalarShadingSocket>(def);
	}
}

///////////////////
std::shared_ptr<FloatSpectralMapSocket> Environment::lookupSpectralMapSocket(
	const Parameter& parameter,
	float def) const
{
	return lookupSpectralMapSocket(parameter, ParametricBlob(def));
}

std::shared_ptr<FloatSpectralMapSocket> Environment::lookupSpectralMapSocket(
	const Parameter& parameter,
	const ParametricBlob& def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstSpectralMapSocket>(def);
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray())
			return std::make_shared<ConstSpectralMapSocket>(def);
		else
			return std::make_shared<ConstSpectralMapSocket>(ParametricBlob(parameter.getNumber(0.0f)));
	case PT_String:
		if (parameter.flags() & PF_Texture) {
			std::string tname = parameter.getString("");

			if (hasMapSocket(tname)) {
				auto socket = getMapSocket(tname);
				if (socket)
					return socket;
			}
		} else {
			std::string sname = parameter.getString("");

			if (hasSpectrum(sname))
				return std::make_shared<ConstSpectralMapSocket>(getSpectrum(sname));
		}
		return std::make_shared<ConstSpectralMapSocket>(def);
	}
}
} // namespace PR
