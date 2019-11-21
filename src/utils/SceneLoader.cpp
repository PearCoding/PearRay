#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "camera/CameraManager.h"
#include "camera/ICamera.h"
#include "camera/ICameraFactory.h"
#include "emission/EmissionManager.h"
#include "emission/IEmission.h"
#include "emission/IEmissionFactory.h"
#include "entity/EntityManager.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "infinitelight/IInfiniteLight.h"
#include "infinitelight/IInfiniteLightFactory.h"
#include "infinitelight/InfiniteLightManager.h"
#include "loader/WavefrontLoader.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "material/MaterialManager.h"
#include "parser/MathParser.h"
#include "parser/MeshParser.h"
#include "parser/SpectralParser.h"
#include "parser/TextureParser.h"
#include "spectral/SpectrumDescriptor.h"

#include "DataLisp.h"

#include <Eigen/SVD>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

/* This source code has many repetitive code lines. Better refactoring needed! */
namespace PR {
std::shared_ptr<Environment> SceneLoader::loadFromFile(const std::wstring& wrkDir,
													   const std::wstring& path,
													   const std::wstring& pluginPath)
{
	std::ifstream stream(encodePath(path));
	std::string str((std::istreambuf_iterator<char>(stream)),
					std::istreambuf_iterator<char>());

	return loadFromString(wrkDir, str, pluginPath);
}

std::shared_ptr<Environment> SceneLoader::loadFromString(const std::wstring& wrkDir,
														 const std::string& source,
														 const std::wstring& pluginPath)
{
	DL::SourceLogger logger;
	DL::DataLisp dataLisp(&logger);
	DL::DataContainer container;

	dataLisp.parse(source);
	dataLisp.build(container);

	auto entries = container.getTopGroups();

	if (entries.empty()) {
		PR_LOG(L_ERROR) << "DataLisp file does not contain valid entries" << std::endl;
		return nullptr;
	} else {
		DL::DataGroup top = entries.front();

		if (top.id() != "scene") {
			PR_LOG(L_ERROR) << "DataLisp file does not contain valid top entry" << std::endl;
			return nullptr;
		} else {
			DL::Data renderWidthD  = top.getFromKey("renderWidth");
			DL::Data renderHeightD = top.getFromKey("renderHeight");
			DL::Data cropD		   = top.getFromKey("crop");
			DL::Data spectrumD	 = top.getFromKey("spectrum");

			std::shared_ptr<SpectrumDescriptor> spectrumDescriptor;
			if (spectrumD.type() == DL::DT_String) {
				std::string spectrum = spectrumD.getString();
				std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(), ::tolower);

				if (spectrum == "xyz") {
					spectrumDescriptor = SpectrumDescriptor::createTriplet();
				} else if (spectrum == "spectral") {
					spectrumDescriptor = SpectrumDescriptor::createStandardSpectral();
				}
			}

			if (!spectrumDescriptor)
				spectrumDescriptor = SpectrumDescriptor::createDefault();

			std::shared_ptr<Environment> env;
			try {
				env = std::make_shared<Environment>(wrkDir, spectrumDescriptor, pluginPath);
			} catch (const BadRenderEnvironment&) {
				return nullptr;
			}

			// Registry information
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);

				if (dataD.type() == DL::DT_Group) {
					DL::DataGroup entry = dataD.getGroup();
					if (entry.id() == "registry") {
						addRegistryEntry(entry, env.get());
					}
				}
			}

			// Output information
			env->outputSpecification().parse(env.get(), top);

			// First independent information
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);

				if (dataD.type() == DL::DT_Group) {
					DL::DataGroup entry = dataD.getGroup();

					if (entry.id() == "spectrum")
						addSpectrum(entry, env.get());
					else if (entry.id() == "texture")
						addTexture(entry, env.get());
					else if (entry.id() == "graph")
						addSubGraph(entry, env.get());
					else if (entry.id() == "mesh")
						addMesh(entry, env.get());
				}
			}

			// Now semi-dependent information
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);

				if (dataD.type() == DL::DT_Group) {
					DL::DataGroup entry = dataD.getGroup();

					if (entry.id() == "material")
						addMaterial(entry, env.get());
					else if (entry.id() == "emission")
						addEmission(entry, env.get());
				}
			}

			// Now entities and lights
			for (size_t i = 0; i < top.anonymousCount(); ++i) {
				DL::Data dataD = top.at(i);

				if (dataD.type() == DL::DT_Group) {
					DL::DataGroup entry = dataD.getGroup();

					if (entry.id() == "entity")
						addEntity(entry, nullptr, env.get());
					else if (entry.id() == "light")
						addLight(entry, env.get());
					else if (entry.id() == "camera")
						addCamera(entry, env.get());
				}
			}

			return env;
		}
	}
}

void SceneLoader::addRegistryEntry(const DL::DataGroup& group, Environment* env)
{
	if (group.anonymousCount() != 2
		|| group.at(0).type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "Invalid registry entry." << std::endl;
		return;
	}

	std::string key = group.at(0).getString();
	DL::Data value  = group.at(1);

	addRegistryEntry(RG_NONE, 0, false, key, value, env);
}

void SceneLoader::setupTransformable(const DL::DataGroup& group,
									 const std::shared_ptr<PR::ITransformable>& entity, Environment* /*env*/)
{
	DL::Data transformD = group.getFromKey("transform");
	DL::Data posD		= group.getFromKey("position");
	DL::Data rotD		= group.getFromKey("rotation");
	DL::Data scaleD		= group.getFromKey("scale");

	if (transformD.type() == DL::DT_Group) {
		bool ok					   = false;
		ITransformable::Transform t = ITransformable::Transform(MathParser::getMatrix(transformD.getGroup(), ok));
		t.makeAffine();

		if (!ok)
			PR_LOG(L_WARNING) << "Couldn't set transform for entity " << entity->name() << std::endl;
		else
			entity->setTransform(t);
	} else {
		bool ok = false;
		Vector3f pos;
		Eigen::Quaternionf rot;
		Vector3f sca;

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
							const std::shared_ptr<PR::ITransformable>& parent, Environment* env)
{
	auto manag		= env->entityManager();
	const uint32 id = manag->nextID();

	DL::Data nameD		= group.getFromKey("name");
	DL::Data typeD		= group.getFromKey("type");
	DL::Data localAreaD = group.getFromKey("local_area");

	populateObjectRegistry(RG_ENTITY, id, group, env);

	std::string name;
	if (nameD.type() == DL::DT_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";

	std::string type;
	if (typeD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "Entity " << name << " couldn't be load. No valid type given." << std::endl;
		return;
	} else {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown entity type " << type << std::endl;
		return;
	}

	auto entity = fac->create(id, id, *env);
	if (!entity) {
		PR_LOG(L_ERROR) << "Could not create entity of type " << type << std::endl;
		return;
	}

	setupTransformable(group, entity, env);

	if (parent) {
		entity->setTransform(parent->transform() * entity->transform());
	}

	if (localAreaD.type() == DL::DT_Bool) {
		if (localAreaD.getBool())
			entity->setFlags(entity->flags() | EF_LocalArea);
		else
			entity->setFlags(entity->flags() & ~(uint8)EF_LocalArea);
	}

	// Add to scene
	manag->addObject(entity);

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		if (group.at(i).type() == DL::DT_Group) {
			DL::DataGroup child = group.at(i).getGroup();

			if (child.id() == "entity")
				addEntity(child, entity, env);
		}
	}
}

void SceneLoader::addCamera(const DL::DataGroup& group, Environment* env)
{
	auto manag		= env->cameraManager();
	const uint32 id = manag->nextID();

	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	populateObjectRegistry(RG_CAMERA, id, group, env);

	std::string name;
	if (nameD.type() == DL::DT_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";

	std::string type;
	if (typeD.type() != DL::DT_String) {
		if (typeD.isValid()) {
			PR_LOG(L_ERROR) << "No valid camera type set" << std::endl;
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
		PR_LOG(L_ERROR) << "Unknown camera type " << type << std::endl;
		return;
	}

	auto camera = fac->create(id, id, *env);
	if (!camera) {
		PR_LOG(L_ERROR) << "Could not create camera of type " << type << std::endl;
		return;
	}

	setupTransformable(group, camera, env);

	manag->addObject(camera);
}

void SceneLoader::addLight(const DL::DataGroup& group, Environment* env)
{
	auto manag		= env->infiniteLightManager();
	const uint32 id = manag->nextID();

	populateObjectRegistry(RG_INFINITELIGHT, id, group, env);

	DL::Data typeD = group.getFromKey("type");

	std::string type;

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "No light type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown light type " << type << std::endl;
		return;
	}

	auto light = fac->create(id, id, *env);
	if (!light) {
		PR_LOG(L_ERROR) << "Could not create light of type " << type << std::endl;
		return;
	}

	setupTransformable(group, light, env);

	env->infiniteLightManager()->addObject(light);
}

void SceneLoader::addEmission(const DL::DataGroup& group, Environment* env)
{
	auto manag		= env->emissionManager();
	const uint32 id = manag->nextID();

	populateObjectRegistry(RG_EMISSION, id, group, env);

	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No emission name set" << std::endl;
		return;
	}

	if (env->hasEmission(name)) {
		PR_LOG(L_ERROR) << "Emission name already set" << std::endl;
		return;
	}

	std::string type;
	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		if (typeD.isValid()) {
			PR_LOG(L_ERROR) << "No valid emission type set" << std::endl;
			return;
		} else {
			type = "diffuse";
		}
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown emission type " << type << std::endl;
		return;
	}

	auto emission = fac->create(id, id, *env);
	if (!emission) {
		PR_LOG(L_ERROR) << "Could not create emission of type " << type << std::endl;
		return;
	}

	env->addEmission(name, emission);
	manag->addObject(emission);
}

void SceneLoader::addMaterial(const DL::DataGroup& group, Environment* env)
{
	auto manag		= env->materialManager();
	const uint32 id = manag->nextID();
	populateObjectRegistry(RG_MATERIAL, id, group, env);

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
		PR_LOG(L_ERROR) << "No material name set" << std::endl;
		return;
	}

	if (env->hasMaterial(name)) {
		PR_LOG(L_ERROR) << "Material name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "No material type set" << std::endl;
		return;
	}

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown material type " << type << std::endl;
		return;
	}

	auto mat = fac->create(id, id, *env);
	if (!mat) {
		PR_LOG(L_ERROR) << "Could not create material of type " << type << std::endl;
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

	env->addMaterial(name, mat);
	manag->addObject(mat);
}

void SceneLoader::addTexture(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD = group.getFromKey("name");

	std::string name;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No texture name set" << std::endl;
		return;
	}

	TextureParser parser;
	parser.parse(env, name, group); // Will be added to env here
}

void SceneLoader::addMesh(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data typeD = group.getFromKey("type");

	std::string name;
	std::string type;

	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No mesh name set" << std::endl;
		return;
	}

	if (env->hasMesh(name)) {
		PR_LOG(L_ERROR) << "Mesh name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		if (typeD.isValid()) {
			PR_LOG(L_ERROR) << "No valid mesh type set" << std::endl;
			return;
		} else {
			type = "triangles";
		}
	}

	auto mesh = MeshParser::parse(env, group);

	if (!mesh) {
		PR_LOG(L_ERROR) << "Mesh " << name << " couldn't be load. Error in " << typeD.getString() << " type parser." << std::endl;
		return;
	}

	env->addMesh(name, mesh);
}

void SceneLoader::addSpectrum(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data dataD = group.getFromKey("data");

	std::string name;
	if (nameD.type() == DL::DT_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "Couldn't get name for spectral entry." << std::endl;
		return;
	}

	if (env->hasSpectrum(name)) {
		PR_LOG(L_ERROR) << "Spectrum name already set" << std::endl;
		return;
	}

	Spectrum spec = SpectralParser::getSpectrum(env->spectrumDescriptor(), dataD);
	env->addSpectrum(name, spec);
}

void SceneLoader::addSubGraph(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD		= group.getFromKey("name");
	DL::Data overridesD = group.getFromKey("overrides");
	DL::Data loaderD	= group.getFromKey("loader");
	DL::Data fileD		= group.getFromKey("file");

	std::string name;
	if (nameD.type() == DL::DT_String)
		name = nameD.getString();

	std::map<std::string, std::string> overrides;
	if (overridesD.type() == DL::DT_String)
		overrides[""] = overridesD.getString();

	std::string file;
	if (fileD.type() == DL::DT_String) {
		file = fileD.getString();
	} else {
		PR_LOG(L_ERROR) << "Couldn't get file for subgraph entry." << std::endl;
		return;
	}

	std::string loader;
	if (loaderD.type() == DL::DT_String) {
		loader = loaderD.getString();
	} else {
		PR_LOG(L_WARNING) << "No valid loader set. Assuming 'obj'." << std::endl;
		loader = "obj";
	}

	if (loader == "obj") {
		DL::Data flipNormalD = group.getFromKey("flipNormal");

		WavefrontLoader loader(overrides);

		if (flipNormalD.type() == DL::DT_Bool)
			loader.flipNormal(flipNormalD.getBool());

		loader.load(file, env);
	} else {
		PR_LOG(L_ERROR) << "Unknown " << loader << " loader." << std::endl;
	}
}

template <typename T>
static void varAddReg(Registry& reg, RegistryGroup regGroup, uint32 uuid, bool hasID,
					  const std::string& key, const T& value)
{
	if (hasID)
		reg.setForObject(regGroup, uuid, key, value);
	else
		reg.setByGroup(regGroup, key, value);
}

void SceneLoader::addRegistryEntry(RegistryGroup regGroup, uint32 uuid, bool hasID,
								   const std::string& key, const DL::Data& value,
								   Environment* env)
{
	Registry& reg = env->registry();

	switch (value.type()) {
	case DL::DT_Integer:
		varAddReg<int64>(reg, regGroup, uuid, hasID, key, (int64)value.getInt());
		break;
	case DL::DT_Float:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getFloat());
		break;
	case DL::DT_Bool:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getBool());
		break;
	case DL::DT_String:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getString());
		break;
	case DL::DT_Group: {
		DL::DataGroup grp = value.getGroup();
		if (grp.isArray() && grp.isAllNumber()) {
			std::vector<float> arr(grp.anonymousCount());
			for (size_t i = 0; i < grp.anonymousCount(); ++i)
				arr[i] = grp.at(i).getNumber();

			varAddReg(reg, regGroup, uuid, hasID, key, arr);
		} else if (grp.isArray()) {
			std::vector<std::string> arr(grp.anonymousCount());
			bool good = true;
			for (size_t i = 0; i < grp.anonymousCount(); ++i) {
				DL::Data entry = grp.at(i);
				if (entry.type() != DL::DT_String) {
					good = false;
					break;
				}
				arr[i] = grp.at(i).getString();
			}

			if (good)
				varAddReg(reg, regGroup, uuid, hasID, key, arr);
			else
				PR_LOG(L_ERROR) << "Invalid registry array type." << std::endl;
		} else if (grp.id() == "texture") {
			if (grp.anonymousCount() == 1 && grp.at(0).type() == DL::DT_String) {
				std::stringstream stream;
				stream << "{t} " << grp.at(0).getString();
				varAddReg(reg, regGroup, uuid, hasID, key, stream.str());
			} else {
				PR_LOG(L_ERROR) << "Invalid texture registry type." << std::endl;
			}
		} else {
			PR_LOG(L_ERROR) << "Invalid registry group type." << std::endl;
		}
	} break;
	default:
		PR_LOG(L_ERROR) << "Invalid registry entry value." << std::endl;
		break;
	}
}

void SceneLoader::populateObjectRegistry(RegistryGroup regGroup, uint32 id,
										 const DL::DataGroup& group, Environment* env)
{
	for (const auto& entry : group.getNamedEntries()) {
		if (entry.key() == "transform"
			|| entry.key() == "position"
			|| entry.key() == "rotation"
			|| entry.key() == "scale")
			continue; // Skip those entries

		if (entry.type() != DL::DT_None)
			addRegistryEntry(regGroup, id, true, entry.key(), entry, env);
	}
}
} // namespace PR
