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
#include "mesh/MeshBase.h"
#include "sampler/SamplerManager.h"
#include "shader/NodeManager.h"
#include "spectral/SpectralMapperManager.h"

#include "parser/CurveParser.h"
#include "parser/MathParser.h"
#include "parser/MeshParser.h"
#include "parser/TextureParser.h"

#include "DataLisp.h"

#include <Eigen/SVD>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace PR {
std::shared_ptr<Environment> SceneLoader::loadFromFile(const std::filesystem::path& path, const LoadOptions& opts)
{
	std::ifstream stream(path.c_str());
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(&stream);
	dataLisp.build(container);

	auto entries = container.getTopGroups();

	SceneLoadContext ctx(path);
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
			DL::Data renderWidthD	 = top.getFromKey("render_width");
			DL::Data renderHeightD	 = top.getFromKey("render_height");
			DL::Data cropD			 = top.getFromKey("crop");
			DL::Data spectralDomainD = top.getFromKey("spectral_domain");
			DL::Data spectralHeroD	 = top.getFromKey("spectral_hero");

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

			if (spectralDomainD.isNumber()) {
				env->renderSettings().spectralStart = spectralDomainD.getNumber();
				env->renderSettings().spectralEnd	= env->renderSettings().spectralStart;
				env->renderSettings().spectralMono	= true;
			} else if (spectralDomainD.type() == DL::DT_Group) {
				const DL::DataGroup sd = spectralDomainD.getGroup();
				if (sd.anonymousCount() == 2 && sd.isAllAnonymousNumber()) {
					env->renderSettings().spectralStart = sd.at(0).getNumber();
					env->renderSettings().spectralEnd	= sd.at(1).getNumber();

					if (env->renderSettings().spectralEnd < env->renderSettings().spectralStart)
						std::swap(env->renderSettings().spectralStart, env->renderSettings().spectralEnd);

					env->renderSettings().spectralMono = env->renderSettings().spectralStart == env->renderSettings().spectralEnd;
				}
			}

			if (spectralHeroD.type() == DL::DT_Bool)
				env->renderSettings().spectralHero = spectralHeroD.getBool();

			env->cache()->setMode(static_cast<CacheMode>(opts.CacheMode));

			std::vector<DL::DataGroup> inner_groups;
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);
				if (dataD.type() == DL::DT_Group)
					inner_groups.push_back(dataD.getGroup());
			}

			ctx.setEnvironment(env.get());
			setupEnvironment(inner_groups, ctx);
			return env;
		}
	}
}

void SceneLoader::setupEnvironment(const std::vector<DL::DataGroup>& groups, SceneLoadContext& ctx)
{
	for (const DL::DataGroup& entry : groups) {
		if (entry.id() == "scene")
			PR_LOG(L_ERROR) << "[Loader] Invalid inner scene entry" << std::endl;
		else if (entry.id() == "include")
			addInclude(entry, ctx);
		else if (entry.id() == "sampler")
			addSampler(entry, ctx);
		else if (entry.id() == "filter")
			addFilter(entry, ctx);
		else if (entry.id() == "integrator")
			addIntegrator(entry, ctx);
		else if (entry.id() == "texture") // Just a sophisticated node
			addTexture(entry, ctx);
		else if (entry.id() == "node")
			addNode(entry, ctx);
		else if (entry.id() == "mesh")
			addMesh(entry, ctx);
		else if (entry.id() == "graph" || entry.id() == "embed")
			addSubGraph(entry, ctx);
		else if (entry.id() == "material")
			addMaterial(entry, ctx);
		else if (entry.id() == "emission")
			addEmission(entry, ctx);
		else if (entry.id() == "entity")
			addEntity(entry, nullptr, ctx);
		else if (entry.id() == "light")
			addLight(entry, ctx);
		else if (entry.id() == "camera")
			addCamera(entry, ctx);
		else if (entry.id() == "spectral_mapper")
			addSpectralMapper(entry, ctx);
		else if (entry.id() == "output")
			ctx.environment()->outputSpecification().parse(ctx.environment(), entry);
	}
}

void SceneLoader::addSampler(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->samplerManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto filter		 = fac->create(id, type, ctx);
	if (!filter) {
		PR_LOG(L_ERROR) << "[Loader] Could not create sampler of type " << type << std::endl;
		return;
	}
	manag->addObject(filter);

	if (slot == "aa" || slot == "pixel" || slot == "antialiasing") {
		if (ctx.environment()->renderSettings().aaSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] AA sampler already selected. Replacing it " << std::endl;

		ctx.environment()->renderSettings().aaSamplerFactory = filter;
	} else if (slot == "lens") {
		if (ctx.environment()->renderSettings().lensSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] Lens sampler already selected. Replacing it " << std::endl;

		ctx.environment()->renderSettings().lensSamplerFactory = filter;
	} else if (slot == "time" || slot == "t") {
		if (ctx.environment()->renderSettings().timeSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] Time sampler already selected. Replacing it " << std::endl;

		ctx.environment()->renderSettings().timeSamplerFactory = filter;
	} else if (slot == "spectral" || slot == "spectrum" || slot == "s") {
		if (ctx.environment()->renderSettings().spectralSamplerFactory)
			PR_LOG(L_WARNING) << "[Loader] Spectral sampler already selected. Replacing it " << std::endl;

		DL::Data rangeD = group.getFromKey("range");
		if (rangeD.type() == DL::DT_Group) {
			DL::DataGroup range = rangeD.getGroup();
			if (range.anonymousCount() == 2 && range.isAllAnonymousNumber()) {
				ctx.environment()->renderSettings().spectralStart = std::min(range.at(0).getNumber(), range.at(1).getNumber());
				ctx.environment()->renderSettings().spectralEnd	  = std::max(range.at(0).getNumber(), range.at(1).getNumber());
			}
		}

		ctx.environment()->renderSettings().spectralSamplerFactory = filter;
	} else {
		PR_LOG(L_ERROR) << "[Loader] Unknown sampler slot " << slot << std::endl;
	}
}

void SceneLoader::addFilter(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->filterManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto filter		 = fac->create(id, type, ctx);
	if (!filter) {
		PR_LOG(L_ERROR) << "[Loader] Could not create filter of type " << type << std::endl;
		return;
	}
	manag->addObject(filter);

	if (slot == "pixel") {
		if (ctx.environment()->renderSettings().pixelFilterFactory)
			PR_LOG(L_WARNING) << "[Loader] Pixel filter already selected. Replacing it " << std::endl;

		ctx.environment()->renderSettings().pixelFilterFactory = filter;
	} else {
		PR_LOG(L_ERROR) << "[Loader] Unknown filter slot " << slot << std::endl;
	}
}

void SceneLoader::addSpectralMapper(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->spectralMapperManager();
	const uint32 id = manag->nextID();

	DL::Data typeD = group.getFromKey("type");

	std::string type;

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "[Loader] No spectral mapper type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown spectral mapper type " << type << std::endl;
		return;
	}

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto mapper		 = fac->create(id, type, ctx);
	if (!mapper) {
		PR_LOG(L_ERROR) << "[Loader] Could not create spectral mapper of type " << type << std::endl;
		return;
	}

	ctx.environment()->spectralMapperManager()->addObject(mapper);
	if (ctx.environment()->renderSettings().spectralMapperFactory)
		PR_LOG(L_WARNING) << "[Loader] Spectral mapper already selected. Replacing it " << std::endl;

	ctx.environment()->renderSettings().spectralMapperFactory = mapper;
}

void SceneLoader::addIntegrator(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->integratorManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto intgr		 = fac->create(id, type, ctx);
	if (!intgr) {
		PR_LOG(L_ERROR) << "[Loader] Could not create integrator of type " << type << std::endl;
		return;
	}

	if (ctx.environment()->renderSettings().integratorFactory)
		PR_LOG(L_WARNING) << "[Loader] Integrator already selected. Replacing it " << std::endl;

	ctx.environment()->renderSettings().integratorFactory = intgr;
}

Transformf SceneLoader::extractTransform(const DL::DataGroup& group)
{
	DL::Data transformD = group.getFromKey("transform");
	DL::Data posD		= group.getFromKey("position");
	DL::Data rotD		= group.getFromKey("rotation");
	DL::Data scaleD		= group.getFromKey("scale");

	if (transformD.type() == DL::DT_Group) {
		bool ok		 = false;
		Transformf t = Transformf(MathParser::getMatrix(transformD.getGroup(), ok));
		t.makeAffine();

		if (!ok)
			PR_LOG(L_WARNING) << "Couldn't set transform " << std::endl;

		return t;
	} else {
		bool ok				   = true;
		Vector3f pos		   = Vector3f(0, 0, 0);
		Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();
		Vector3f sca		   = Vector3f(1, 1, 1);

		if (posD.type() == DL::DT_Group) {
			pos = MathParser::getVector(posD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set position " << std::endl;
		}

		if (ok && rotD.type() == DL::DT_Group) {
			rot = MathParser::getRotation(rotD, ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set rotation " << std::endl;
		}

		if (ok && scaleD.isNumber()) {
			float s = scaleD.getNumber();
			sca		= Vector3f(s, s, s);
		} else if (ok && scaleD.type() == DL::DT_Group) {
			sca = MathParser::getVector(scaleD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set scale " << std::endl;
		}

		if (!ok) {
			return Transformf::Identity();
		} else {
			rot.normalize();

			Transformf trans;
			trans.fromPositionOrientationScale(pos, rot, sca);
			trans.makeAffine();

			return trans;
		}
	}
}

void SceneLoader::addEntity(const DL::DataGroup& group,
							const std::shared_ptr<PR::ITransformable>& parent, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->entityManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	if (parent)
		ctx.transform() = parent->transform() * extractTransform(group);
	else
		ctx.transform() = extractTransform(group);

	auto entity = fac->create(id, type, ctx);
	if (!entity) {
		PR_LOG(L_ERROR) << "[Loader] Could not create entity of type " << type << std::endl;
		return;
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
	auto manag		= ctx.environment()->cameraManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	ctx.transform()	 = extractTransform(group);
	auto camera		 = fac->create(id, type, ctx);
	if (!camera) {
		PR_LOG(L_ERROR) << "[Loader] Could not create camera of type " << type << std::endl;
		return;
	}

	manag->addObject(camera);
}

void SceneLoader::addLight(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->infiniteLightManager();
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

	ctx.parameters() = populateObjectParameters(group, ctx);
	ctx.transform()	 = extractTransform(group);
	auto light		 = fac->create(id, type, ctx);
	if (!light) {
		PR_LOG(L_ERROR) << "[Loader] Could not create light of type " << type << std::endl;
		return;
	}

	ctx.environment()->infiniteLightManager()->addObject(light);
}

void SceneLoader::addEmission(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No emission name set" << std::endl;
		return;
	}

	if (ctx.environment()->hasEmission(name)) {
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

	ctx.registerEmission(name, type, populateObjectParameters(group, ctx));
}

void SceneLoader::addMaterial(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	std::string type;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No material name set" << std::endl;
		return;
	}

	if (ctx.environment()->hasMaterial(name)) {
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

	ctx.registerMaterial(name, type, populateObjectParameters(group, ctx));
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

	TextureParser::parse(ctx, name, group); // Will be added to ctx here
}

void SceneLoader::addNode(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->nodeManager();
	const uint32 id = manag->nextID();

	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	std::string type;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "[Loader] No node name set" << std::endl;
		return;
	}

	if (ctx.environment()->hasNode(name)) {
		PR_LOG(L_ERROR) << "[Loader] Node name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "[Loader] No node type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown node type " << type << std::endl;
		return;
	}

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto node		 = fac->create(id, type, ctx);
	if (!node) {
		PR_LOG(L_ERROR) << "[Loader] Could not create node of type " << type << std::endl;
		return;
	}

	ctx.environment()->addNode(name, node);
	manag->addObject(node);
}

// Assume node from the groups id
uint32 SceneLoader::addNodeInline(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	auto manag		= ctx.environment()->nodeManager();
	const uint32 id = manag->nextID();

	std::string type = group.id();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "[Loader] Unknown node type " << type << std::endl;
		return P_INVALID_REFERENCE;
	}

	ctx.parameters() = populateObjectParameters(group, ctx);
	auto node		 = fac->create(id, type, ctx);
	if (!node) {
		PR_LOG(L_ERROR) << "[Loader] Could not create node of type " << type << std::endl;
		return P_INVALID_REFERENCE;
	}

	return manag->addObject(node);
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

	if (ctx.environment()->hasMesh(name)) {
		PR_LOG(L_ERROR) << "[Loader] Mesh name already set" << std::endl;
		return;
	}

	ctx.environment()->cache()->unloadAll(); // Make sure there is enough space

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

	ctx.environment()->addMesh(name, std::move(mesh));
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

	ctx.environment()->cache()->unloadAll(); // Make sure there is enough space
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
	std::filesystem::path real_path = ctx.escapePath(path);
	PR_LOG(L_DEBUG) << "[Loader] Including " << real_path << std::endl;
	if (ctx.hasFile(real_path)) {
		PR_LOG(L_ERROR) << "[Loader] Include file " << real_path << " already included! " << std::endl;
		return;
	}

	std::ifstream stream(real_path.c_str());
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(&stream);
	dataLisp.build(container);

	if (container.getTopGroups().empty()) {
		PR_LOG(L_WARNING) << "[Loader] Include file " << real_path << " is empty! " << std::endl;
		return;
	}

	ctx.pushFile(real_path);
	setupEnvironment(container.getTopGroups(), ctx);
	ctx.popFile();
}

ParameterGroup SceneLoader::populateObjectParameters(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	ParameterGroup params;
	// Named parameters
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
				} else if (grp.isAllAnonymousNumber()) {
					std::vector<float> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getNumber();
					params.addParameter(entry.key(), Parameter::fromNumberArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_String)) {
					std::vector<std::string> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getString();
					params.addParameter(entry.key(), Parameter::fromStringArray(arr));
				} else {
					PR_LOG(L_ERROR) << "[Loader] Array inner type mismatch" << std::endl;
				}
			} else {
				Parameter param = unpackShadingNetwork(grp, ctx);
				if (param.isValid())
					params.addParameter(entry.key(), param);
			}
		} break;
		default:
			PR_LOG(L_ERROR) << "[Loader] Invalid parameter entry value." << std::endl;
			break;
		}
	}

	// Positional parameters
	for (const auto& entry : group.getAnonymousEntries()) {
		switch (entry.type()) {
		case DL::DT_Integer:
			params.addParameter(Parameter::fromInt(entry.getInt()));
			break;
		case DL::DT_Float:
			params.addParameter(Parameter::fromNumber(entry.getNumber()));
			break;
		case DL::DT_Bool:
			params.addParameter(Parameter::fromBool(entry.getBool()));
			break;
		case DL::DT_String:
			params.addParameter(Parameter::fromString(entry.getString()));
			break;
		case DL::DT_Group: {
			DL::DataGroup grp = entry.getGroup();
			if (grp.isArray()) {
				if (grp.isAllAnonymousOfType(DL::DT_Bool)) {
					std::vector<bool> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getBool();
					params.addParameter(Parameter::fromBoolArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_Integer)) {
					std::vector<int64> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getInt();
					params.addParameter(Parameter::fromIntArray(arr));
				} else if (grp.isAllAnonymousNumber()) {
					std::vector<float> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getNumber();
					params.addParameter(Parameter::fromNumberArray(arr));
				} else if (grp.isAllAnonymousOfType(DL::DT_String)) {
					std::vector<std::string> arr(grp.anonymousCount());
					for (size_t i = 0; i < grp.anonymousCount(); ++i)
						arr[i] = grp.at(i).getString();
					params.addParameter(Parameter::fromStringArray(arr));
				} else {
					PR_LOG(L_ERROR) << "[Loader] Array inner type mismatch" << std::endl;
				}
			} else {
				Parameter param = unpackShadingNetwork(grp, ctx);
				if (param.isValid())
					params.addParameter(param);
			}
		} break;
		default:
			PR_LOG(L_ERROR) << "[Loader] Invalid parameter entry value." << std::endl;
			break;
		}
	}
	return params;
}

Parameter SceneLoader::unpackShadingNetwork(const DL::DataGroup& group, SceneLoadContext& ctx)
{
	if (group.id() == "texture") {
		if (group.anonymousCount() == 1 && group.at(0).type() == DL::DT_String) {
			auto node = ctx.environment()->getRawNode(group.at(0).getString());
			if (node) {
				uint32 id = ctx.environment()->nodeManager()->addObject(node);
				return Parameter::fromReference(id);
			} else {
				PR_LOG(L_ERROR) << "[Loader] Unknown texture " << group.at(0).getString() << std::endl;
			}
		} else {
			PR_LOG(L_ERROR) << "[Loader] Invalid texture parameter" << std::endl;
		}
	} else if (group.id() == "node") {
		if (group.anonymousCount() == 1 && group.at(0).type() == DL::DT_String) {
			auto node = ctx.environment()->getRawNode(group.at(0).getString());
			if (node) {
				uint32 id = ctx.environment()->nodeManager()->addObject(node);
				return Parameter::fromReference(id);
			} else {
				PR_LOG(L_ERROR) << "[Loader] Unknown node " << group.at(0).getString() << std::endl;
			}
		} else {
			PR_LOG(L_ERROR) << "[Loader] Invalid node parameter" << std::endl;
		}
	} else if (group.id() == "deg2rad") { // TODO: A custom plugin based approach would be great
		if (group.anonymousCount() == 1 && group.at(0).isNumber())
			return Parameter::fromNumber(group.at(0).getNumber() * PR_DEG2RAD);
		else
			PR_LOG(L_ERROR) << "[Loader] Invalid node parameter" << std::endl;
	} else if (group.id() == "rad2deg") {
		if (group.anonymousCount() == 1 && group.at(0).isNumber())
			return Parameter::fromNumber(group.at(0).getNumber() * PR_RAD2DEG);
		else
			PR_LOG(L_ERROR) << "[Loader] Invalid node parameter" << std::endl;
	} else {
		uint32 id = addNodeInline(group, ctx);
		if (id != P_INVALID_REFERENCE)
			return Parameter::fromReference(id);
	}
	return Parameter();
}
} // namespace PR
