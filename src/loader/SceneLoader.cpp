#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "archives/PlyLoader.h"
#include "archives/WavefrontLoader.h"
#include "cache/Cache.h"
#include "camera/CameraManager.h"
#include "camera/ICamera.h"
#include "camera/ICameraPlugin.h"
#include "emission/EmissionManager.h"
#include "emission/IEmission.h"
#include "emission/IEmissionPlugin.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "filter/FilterManager.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightPlugin.h"
#include "infinitelight/InfiniteLightManager.h"
#include "integrator/IntegratorManager.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "material/MaterialManager.h"
#include "mesh/MeshFactory.h"

#include "parser/CurveParser.h"
#include "parser/MathParser.h"
#include "parser/MeshParser.h"
#include "parser/SpectralParser.h"
#include "parser/TextureParser.h"
#include "sampler/SamplerManager.h"

#include "DataLisp.h"

#include <Eigen/SVD>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

namespace PR {
std::shared_ptr<Environment> SceneLoader::loadFromFile(const std::wstring& path, const LoadOptions& opts)
{
	std::ifstream stream(encodePath(path));
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(&stream);
	dataLisp.build(container);

	auto entries = container.getTopGroups();

	SceneLoadContext ctx;
	ctx.FileStack.push_back(path);
	return createEnvironment(entries, opts, ctx);
}

std::shared_ptr<Environment> SceneLoader::loadFromString(const std::string& source, const LoadOptions& opts)
{
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(source);
	dataLisp.build(container);

	auto entries = container.getTopGroups();

	SceneLoadContext ctx;
	return createEnvironment(entries, opts, ctx);
}

std::shared_ptr<Environment> SceneLoader::createEnvironment(const std::vector<DL::DataGroup>& groups,
															const LoadOptions& opts,
															SceneLoadContext& ctx)
{
	if (groups.empty()) {
		PR_LOG(L_ERROR) << "DataLisp file does not contain valid entries" << std::endl;
		return nullptr;
	} else {
		DL::DataGroup top = groups.front();

		if (top.id() != "scene") {
			PR_LOG(L_ERROR) << "DataLisp file does not contain valid top entry" << std::endl;
			return nullptr;
		} else {
			DL::Data renderWidthD  = top.getFromKey("renderWidth");
			DL::Data renderHeightD = top.getFromKey("renderHeight");
			DL::Data cropD		   = top.getFromKey("crop");

			std::shared_ptr<Environment> env;
			try {
				env = std::make_shared<Environment>(opts.WorkingDir, opts.PluginPath);
			} catch (const BadRenderEnvironment&) {
				return nullptr;
			}

			if (renderWidthD.type() == DL::DT_Integer)
				env->renderSettings().filmWidth = renderWidthD.getInt();

			if (renderHeightD.type() == DL::DT_Integer)
				env->renderSettings().filmHeight = renderHeightD.getInt();

			if (cropD.type() == DL::DT_Group) {
				DL::DataGroup crop = cropD.getGroup();
				if (crop.anonymousCount() == 4 && crop.isAllAnonymousNumber()) {
					env->renderSettings().cropMinX = crop.at(0).getNumber();
					env->renderSettings().cropMaxX = crop.at(1).getNumber();
					env->renderSettings().cropMinY = crop.at(2).getNumber();
					env->renderSettings().cropMaxY = crop.at(3).getNumber();
				}
			}

			env->cache()->setMode(static_cast<CacheMode>(opts.CacheMode));

			std::vector<DL::DataGroup> inner_groups;
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);
				if (dataD.type() == DL::DT_Group)
					inner_groups.push_back(dataD.getGroup());
			}

			ctx.Env = env.get();
			setupEnvironment(inner_groups, ctx);
			return env;
		}
	}
}

void SceneLoader::setupEnvironment(const std::vector<DL::DataGroup>& groups, SceneLoadContext& ctx)
{
	// Output information
	ctx.Env->outputSpecification().parse(ctx.Env, groups);

	// Includees
	for (const DL::DataGroup& entry : groups) {
		if (entry.id() == "scene") {
			PR_LOG(L_ERROR) << "[Loader] Invalid inner scene entry" << std::endl;
		} else if (entry.id() == "include")
			addInclude(entry, ctx);
	}

	// Independent information
	for (const DL::DataGroup& entry : groups) {
		if (entry.id() == "spectrum")
			addSpectrum(entry, ctx);
		else if (entry.id() == "sampler")
			addSampler(entry, ctx);
		else if (entry.id() == "filter")
			addFilter(entry, ctx);
		else if (entry.id() == "integrator")
			addIntegrator(entry, ctx);
		else if (entry.id() == "texture")
			addTexture(entry, ctx);
		else if (entry.id() == "mesh")
			addMesh(entry, ctx);
		else if (entry.id() == "graph" || entry.id() == "embed")
			addSubGraph(entry, ctx);
	}

	// Now semi-dependent information
	for (const DL::DataGroup& entry : groups) {
		if (entry.id() == "material")
			addMaterial(entry, ctx);
		else if (entry.id() == "emission")
			addEmission(entry, ctx);
	}

	// Now entities and lights
	for (const DL::DataGroup& entry : groups) {
		if (entry.id() == "entity")
			addEntity(entry, nullptr, ctx);
		else if (entry.id() == "light")
			addLight(entry, ctx);
		else if (entry.id() == "camera")
			addCamera(entry, ctx);
	}
}

void SceneLoader::addSampler(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->samplerManager();
	const uint32 id = manag->nextID();

	DL::Data typeD = group.getFromKey("type");
	std::string type;
	if (typeD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "[Loader] Sampler could not be load. No valid type given." << std::endl;
		return;
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	DL::Data slotD = group.getFromKey("slot");
	std::string slot;
	if (slotD.type() != DL::DT_String) {
		slot = "aa";
	} else {
		slot = slotD.getString();
		std::transform(slot.begin(), slot.end(), slot.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown sampler type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto filter	   = fac->create(id, ctx);
	if (!filter) {
		PR_LOG(L_ERROR) << "[Loader] Could not create sampler of type " << type << std::endl;
		return;
	}
	manag->addObject(filter);

	if (slot == "aa" || slot == "pixel" || slot == "antialiasing") {
		if (ctx.Env->renderSettings().aaSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] AA sampler already selected. Replacing it " << std::endl;

		ctx.Env->renderSettings().aaSamplerFactory = filter;
	} else if (slot == "lens") {
		if (ctx.Env->renderSettings().lensSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] Lens sampler already selected. Replacing it " << std::endl;

		ctx.Env->renderSettings().lensSamplerFactory = filter;
	} else if (slot == "time" || slot == "t") {
		if (ctx.Env->renderSettings().timeSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] Time sampler already selected. Replacing it " << std::endl;

		ctx.Env->renderSettings().timeSamplerFactory = filter;
	} else {
		PR_LOG(L_ERROR) << "[Loader] Unknown sampler slot " << slot << std::endl;
	}
}

void SceneLoader::addFilter(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->filterManager();
	const uint32 id = manag->nextID();

	DL::Data typeD = group.getFromKey("type");
	std::string type;
	if (typeD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "[Loader] Filter could not be load. No valid type given." << std::endl;
		return;
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	DL::Data slotD = group.getFromKey("slot");
	std::string slot;
	if (slotD.type() != DL::DT_String) {
		slot = "pixel";
	} else {
		slot = slotD.getString();
		std::transform(slot.begin(), slot.end(), slot.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown filter type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto filter	   = fac->create(id, ctx);
	if (!filter) {
		PR_LOG(L_ERROR) << "[Loader] Could not create filter of type " << type << std::endl;
		return;
	}
	manag->addObject(filter);

	if (slot == "pixel") {
		if (ctx.Env->renderSettings().pixelFilterFactory)
			PR_LOG(L_WARNING) << "[Loader] Pixel filter already selected. Replacing it " << std::endl;

		ctx.Env->renderSettings().pixelFilterFactory = filter;
	} else {
		PR_LOG(L_ERROR) << "[Loader] Unknown filter slot " << slot << std::endl;
	}
}

void SceneLoader::addIntegrator(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->integratorManager();
	const uint32 id = manag->nextID();

	DL::Data typeD = group.getFromKey("type");
	std::string type;
	if (typeD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "[Loader] Integrator could not be load. No valid type given." << std::endl;
		return;
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown integrator type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto intgr	   = fac->create(id, ctx);
	if (!intgr) {
		PR_LOG(L_ERROR) << "[Loader] Could not create integrator of type " << type << std::endl;
		return;
	}

	if (ctx.Env->renderSettings().integratorFactory)
		PR_LOG(L_WARNING) << "[Loader] Integrator already selected. Replacing it " << std::endl;

	ctx.Env->renderSettings().integratorFactory = intgr;
}

void SceneLoader::setupTransformable(const DL::DataGroup& group,
									 const std::shared_ptr<PR::ITransformable>& entity, SceneLoadContext& /*ctx*/)
{
	DL::Data transformD = group.getFromKey("transform");
	DL::Data posD		= group.getFromKey("position");
	DL::Data rotD		= group.getFromKey("rotation");
	DL::Data scaleD		= group.getFromKey("scale");

	if (transformD.type() == DL::DT_Group) {
		bool ok						= false;
		ITransformable::Transform t = ITransformable::Transform(MathParser::getMatrix(transformD.getGroup(), ok));
		t.makeAffine();

		if (!ok)
			PR_LOG(L_WARNING) << "Couldn't set transform for entity " << entity->name() << std::endl;
		else
			entity->setTransform(t);
	} else {
		bool ok				   = true;
		Vector3f pos		   = Vector3f(0, 0, 0);
		Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();
		Vector3f sca		   = Vector3f(1, 1, 1);

		if (posD.type() == DL::DT_Group) {
			pos = MathParser::getVector(posD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set position for entity " << entity->name() << std::endl;
		}

		if (ok && rotD.type() == DL::DT_Group) {
			rot = MathParser::getRotation(rotD, ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set rotation for entity " << entity->name() << std::endl;
		}

		if (ok && scaleD.isNumber()) {
			float s = scaleD.getNumber();
			sca		= Vector3f(s, s, s);
		} else if (ok && scaleD.type() == DL::DT_Group) {
			sca = MathParser::getVector(scaleD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set scale for entity " << entity->name() << std::endl;
		}

		if (!ok) {
			return;
		} else {
			rot.normalize();

			ITransformable::Transform trans;
			trans.fromPositionOrientationScale(pos, rot, sca);
			trans.makeAffine();

			entity->setTransform(trans);
		}
	}
}

void SceneLoader::addEntity(const DL::DataGroup& group,
							const std::shared_ptr<PR::ITransformable>& parent, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->entityManager();
	const uint32 id = manag->nextID();

	DL::Data nameD			= group.getFromKey("name");
	DL::Data typeD			= group.getFromKey("type");
	DL::Data localAreaD		= group.getFromKey("local_area");
	DL::Data cameraVisibleD = group.getFromKey("camera_visible");
	DL::Data lightVisibleD	= group.getFromKey("light_visible");
	DL::Data bounceVisibleD = group.getFromKey("bounce_visible");
	DL::Data shadowVisibleD = group.getFromKey("shadow_visible");

	std::string name;
	if (nameD.type() == DL::DT_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";

	std::string type;
	if (typeD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "[Loader] Entity " << name << " couldn't be load. No valid type given." << std::endl;
		return;
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown entity type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto entity	   = fac->create(id, ctx);
	if (!entity) {
		PR_LOG(L_ERROR) << "[Loader] Could not create entity of type " << type << std::endl;
		return;
	}

	setupTransformable(group, entity, ctx);

	if (parent) {
		entity->setTransform(parent->transform() * entity->transform());
	}

	if (localAreaD.type() == DL::DT_Bool) {
		if (localAreaD.getBool())
			entity->setFlags(entity->flags() | EF_LocalArea);
		else
			entity->setFlags(entity->flags() & ~(uint8)EF_LocalArea);
	}

	uint8 visFlags = 0;
	visFlags |= (cameraVisibleD.type() != DL::DT_Bool || cameraVisibleD.getBool()) ? (uint8)EVF_Camera : 0;
	visFlags |= (lightVisibleD.type() != DL::DT_Bool || lightVisibleD.getBool()) ? (uint8)EVF_Light : 0;
	visFlags |= (bounceVisibleD.type() != DL::DT_Bool || bounceVisibleD.getBool()) ? (uint8)EVF_Bounce : 0;
	visFlags |= (shadowVisibleD.type() != DL::DT_Bool || shadowVisibleD.getBool()) ? (uint8)EVF_Shadow : 0;
	entity->setVisibilityFlags(visFlags);

	// Add to scene
	manag->addObject(entity);

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		if (group.at(i).type() == DL::DT_Group) {
			DL::DataGroup child = group.at(i).getGroup();

			if (child.id() == "entity")
				addEntity(child, entity, ctx);
		}
	}
}

void SceneLoader::addCamera(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->cameraManager();
	const uint32 id = manag->nextID();

	//DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	/*std::string name;
	if (nameD.type() == DL::DT_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";*/

	std::string type;
	if (typeD.type() != DL::DT_String) {
		if (typeD.isValid()) {
			PR_LOG(L_ERROR) << "[Loader] No valid camera type set" << std::endl;
			return;
		} else {
			type = "standard";
		}
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown camera type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto camera	   = fac->create(id, ctx);
	if (!camera) {
		PR_LOG(L_ERROR) << "[Loader] Could not create camera of type " << type << std::endl;
		return;
	}

	setupTransformable(group, camera, ctx);

	manag->addObject(camera);
}

void SceneLoader::addLight(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->infiniteLightManager();
	const uint32 id = manag->nextID();

	DL::Data typeD = group.getFromKey("type");

	std::string type;

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "[Loader] No light type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown light type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto light	   = fac->create(id, ctx);
	if (!light) {
		PR_LOG(L_ERROR) << "[Loader] Could not create light of type " << type << std::endl;
		return;
	}

	setupTransformable(group, light, ctx);

	ctx.Env->infiniteLightManager()->addObject(light);
}

void SceneLoader::addEmission(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->emissionManager();
	const uint32 id = manag->nextID();

	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No emission name set" << std::endl;
		return;
	}

	if (ctx.Env->hasEmission(name)) {
		PR_LOG(L_ERROR) << "[Loader] Emission name already set" << std::endl;
		return;
	}

	std::string type;
	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		if (typeD.isValid()) {
			PR_LOG(L_ERROR) << "[Loader] No valid emission type set" << std::endl;
			return;
		} else {
			type = "diffuse";
		}
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown emission type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto emission  = fac->create(id, ctx);
	if (!emission) {
		PR_LOG(L_ERROR) << "[Loader] Could not create emission of type " << type << std::endl;
		return;
	}

	ctx.Env->addEmission(name, emission);
	manag->addObject(emission);
}

void SceneLoader::addMaterial(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.Env->materialManager();
	const uint32 id = manag->nextID();

	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	DL::Data shadowD		= group.getFromKey("shadow");
	DL::Data selfShadowD	= group.getFromKey("self_shadow");
	DL::Data cameraVisibleD = group.getFromKey("camera_visible");
	DL::Data shadeableD		= group.getFromKey("shadeable");

	std::string name;
	std::string type;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No material name set" << std::endl;
		return;
	}

	if (ctx.Env->hasMaterial(name)) {
		PR_LOG(L_ERROR) << "[Loader] Material name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "[Loader] No material type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown material type " << type << std::endl;
		return;
	}

	ctx.Parameters = populateObjectParameters(group);
	auto mat	   = fac->create(id, ctx);
	if (!mat) {
		PR_LOG(L_ERROR) << "[Loader] Could not create material of type " << type << std::endl;
		return;
	}

	if (shadeableD.type() == DL::DT_Bool)
		mat->enableShading(shadeableD.getBool());

	if (shadowD.type() == DL::DT_Bool)
		mat->enableShadow(shadowD.getBool());

	if (selfShadowD.type() == DL::DT_Bool)
		mat->enableSelfShadow(selfShadowD.getBool());

	if (cameraVisibleD.type() == DL::DT_Bool)
		mat->enableCameraVisibility(cameraVisibleD.getBool());

	ctx.Env->addMaterial(name, mat);
	manag->addObject(mat);
}

void SceneLoader::addTexture(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data nameD = group.getFromKey("name");

	std::string name;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No texture name set" << std::endl;
		return;
	}

	TextureParser parser;
	parser.parse(ctx.Env, name, group); // Will be added to ctx here
}

void SceneLoader::addMesh(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data nameD = group.getFromKey("name");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No mesh name set" << std::endl;
		return;
	}

	if (ctx.Env->hasMesh(name)) {
		PR_LOG(L_ERROR) << "[Loader] Mesh name already set" << std::endl;
		return;
	}

	ctx.Env->cache()->unloadAll(); // Make sure there is enough space

	std::unique_ptr<MeshBase> mesh;
	try {
		mesh = MeshParser::parse(group);
	} catch (const std::bad_alloc& ex) {
		PR_LOG(L_ERROR) << "[Loader] Out of memory to load mesh " << name << ": " << ex.what() << std::endl;
		return;
	}

	if (!mesh) {
		PR_LOG(L_ERROR) << "[Loader] Mesh " << name << " could not be load" << std::endl;
		return;
	}

	if (!mesh->isOnlyTriangular()) {
		PR_LOG(L_WARNING) << "[Loader] Mesh " << name << " has to be triangulated" << std::endl;
		mesh->triangulate();
	}

	bool useCache	   = ctx.Env->cache()->shouldCacheMesh(mesh->nodeCount());
	DL::Data useCacheD = group.getFromKey("cache");
	if (useCacheD.type() == DL::DT_Bool)
		useCache = useCacheD.getBool();

	ctx.Env->addMesh(name, MeshFactory::create(name, std::move(mesh), ctx.Env->cache(), useCache));
}

void SceneLoader::addSpectrum(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data dataD = group.getFromKey("data");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] Couldn't get name for spectral entry." << std::endl;
		return;
	}

	if (ctx.Env->hasSpectrum(name)) {
		PR_LOG(L_ERROR) << "[Loader] Spectrum name already set" << std::endl;
		return;
	}

	// TODO
	const auto spec = SpectralParser::getSpectrum(ctx.Env->defaultSpectralUpsampler().get(), dataD);
	ctx.Env->addSpectrum(name, spec);
}

void SceneLoader::addSubGraph(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data loaderD = group.getFromKey("loader");
	DL::Data fileD	 = group.getFromKey("file");

	std::wstring file;
	if (fileD.type() == DL::DT_String) {
		std::string f = fileD.getString();
		file		  = std::wstring(f.begin(), f.end());
	} else {
		PR_LOG(L_ERROR) << "[Loader] Could not get file for subgraph entry." << std::endl;
		return;
	}

	std::string loader;
	if (loaderD.type() == DL::DT_String) {
		loader = loaderD.getString();
	} else {
		PR_LOG(L_WARNING) << "[Loader] No valid loader set. Assuming 'obj'." << std::endl;
		loader = "obj";
	}

	ctx.Env->cache()->unloadAll(); // Make sure there is enough space
	if (loader == "obj") {
		DL::Data nameD		 = group.getFromKey("name");
		DL::Data flipNormalD = group.getFromKey("flipNormal");
		DL::Data cacheD		 = group.getFromKey("cache");

		WavefrontLoader loader(nameD.type() == DL::DT_String ? nameD.getString() : "");

		if (flipNormalD.type() == DL::DT_Bool)
			loader.flipNormal(flipNormalD.getBool());

		if (cacheD.type() == DL::DT_Bool)
			loader.setCacheMode(cacheD.getBool() ? CM_All : CM_None);

		try {
			loader.load(file, ctx);
		} catch (const std::bad_alloc& ex) {
			PR_LOG(L_ERROR) << "[Loader] Out of memory to load subgraph " << fileD.getString() << std::endl;
		}
	} else if (loader == "ply") {
		DL::Data flipNormalD = group.getFromKey("flipNormal");
		DL::Data nameD		 = group.getFromKey("name");
		DL::Data cacheD		 = group.getFromKey("cache");

		PlyLoader loader(nameD.type() == DL::DT_String ? nameD.getString() : "");

		if (flipNormalD.type() == DL::DT_Bool)
			loader.flipNormal(flipNormalD.getBool());

		if (cacheD.type() == DL::DT_Bool)
			loader.setCacheMode(cacheD.getBool() ? CM_All : CM_None);

		try {
			loader.load(file, ctx);
		} catch (const std::bad_alloc& ex) {
			PR_LOG(L_ERROR) << "[Loader] Out of memory to load subgraph " << fileD.getString() << std::endl;
		}
	} else {
		PR_LOG(L_ERROR) << "[Loader] Unknown " << loader << " loader." << std::endl;
	}
}

void SceneLoader::addInclude(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	if (group.anonymousCount() == 1 && group.isAllAnonymousOfType(DL::DT_String)) {
		include(group.at(0).getString(), ctx);
	} else {
		PR_LOG(L_ERROR) << "[Loader] Invalid include directive." << std::endl;
	}
}

void SceneLoader::include(const std::string& path, SceneLoadContext& ctx)
{
	PR_LOG(L_DEBUG) << "[Loader] Including " << path << std::endl;
	const std::wstring wpath = boost::filesystem::path(path).generic_wstring();
	if (std::find(ctx.FileStack.begin(), ctx.FileStack.end(), wpath) != ctx.FileStack.end()) {
		PR_LOG(L_ERROR) << "[Loader] Include file " << path << " already included! " << std::endl;
		return;
	}

	std::ifstream stream(encodePath(path));
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(&stream);
	dataLisp.build(container);

	ctx.FileStack.push_back(wpath);
	setupEnvironment(container.getTopGroups(), ctx);
	PR_ASSERT(ctx.FileStack.size() >= 1, "Invalid push/pop count");
	ctx.FileStack.pop_back();
}

ParameterGroup SceneLoader::populateObjectParameters(const DL::DataGroup& group)
{
	ParameterGroup params;
	for (const auto& entry : group.getNamedEntries()) {
		switch (entry.type()) {
		case DL::DT_Integer:
			params.addParameter(entry.key(), Parameter::fromInt(entry.getInt()));
			break;
		case DL::DT_Float:
			params.addParameter(entry.key(), Parameter::fromNumber(entry.getNumber()));
			break;
		case DL::DT_Bool:
			params.addParameter(entry.key(), Parameter::fromBool(entry.getBool()));
			break;
		case DL::DT_String:
			params.addParameter(entry.key(), Parameter::fromString(entry.getString()));
			break;
		case DL::DT_Group: {
			DL::DataGroup grp = entry.getGroup();
			if (grp.isArray()) {
				if (grp.isAllAnonymousOfType(DL::DT_Bool)) {
					std::vector<bool> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getBool();
					params.addParameter(entry.key(), Parameter::fromBoolArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_Integer)) {
					std::vector<int64> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getInt();
					params.addParameter(entry.key(), Parameter::fromIntArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_Float)) {
					std::vector<float> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getFloat();
					params.addParameter(entry.key(), Parameter::fromNumberArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_String)) {
					std::vector<std::string> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getString();
					params.addParameter(entry.key(), Parameter::fromStringArray(arr));
				} else {
					PR_LOG(L_ERROR) << "[Loader] Array inner type mismatch" << std::endl;
				}
			} else if (grp.id() == "texture") {
				if (grp.anonymousCount() == 1 && grp.at(0).type() == DL::DT_String) {
					Parameter param = Parameter::fromString(grp.at(0).getString());
					param.assignFlags(PF_Texture);
					params.addParameter(entry.key(), param);
				} else {
					PR_LOG(L_ERROR) << "[Loader] Invalid texture parameter" << std::endl;
				}
			} else {
				PR_LOG(L_ERROR) << "[Loader] Invalid parameter group type: " << grp.id() << std::endl;
			}
		} break;
		default:
			PR_LOG(L_ERROR) << "[Loader] Invalid parameter entry value." << std::endl;
			break;
		}
	}
	return params;
}
} // namespace PR
