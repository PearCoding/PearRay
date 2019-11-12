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
#include "mesh/TriMesh.h"
#include "parser/mesh/TriMeshInlineParser.h"
#include "parser/texture/TextureParser.h"
#include "shader/ConstSocket.h"
#include "spectral/RGBConverter.h"
#include "spectral/SpectrumDescriptor.h"
#include "spectral/XYZConverter.h"

#include "DataLisp.h"

#include <Eigen/SVD>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

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
			if (spectrumD.type() == DL::Data::T_String) {
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

				if (dataD.type() == DL::Data::T_Group) {
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

				if (dataD.type() == DL::Data::T_Group) {
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

				if (dataD.type() == DL::Data::T_Group) {
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

				if (dataD.type() == DL::Data::T_Group) {
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
		|| group.at(0).type() != DL::Data::T_String) {
		PR_LOG(L_ERROR) << "Invalid registry entry." << std::endl;
		return;
	}

	std::string key = group.at(0).getString();
	DL::Data value  = group.at(1);

	addRegistryEntry(RG_NONE, 0, false, key, value, env);
}

void SceneLoader::setupVirtualEntity(const DL::DataGroup& group,
									 const std::shared_ptr<PR::VirtualEntity>& entity, Environment* /*env*/)
{
	DL::Data transformD = group.getFromKey("transform");
	DL::Data posD		= group.getFromKey("position");
	DL::Data rotD		= group.getFromKey("rotation");
	DL::Data scaleD		= group.getFromKey("scale");

	if (transformD.type() == DL::Data::T_Group) {
		bool ok					   = false;
		VirtualEntity::Transform t = VirtualEntity::Transform(getMatrix(transformD.getGroup(), ok));
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

		if (posD.type() == DL::Data::T_Group) {
			pos = getVector(posD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set position for entity " << entity->name() << std::endl;
		}

		if (ok && rotD.type() == DL::Data::T_Group) {
			rot = getRotation(rotD, ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set rotation for entity " << entity->name() << std::endl;
		}

		if (ok && scaleD.isNumber()) {
			float s = scaleD.getNumber();
			sca		= Vector3f(s, s, s);
		} else if (ok && scaleD.type() == DL::Data::T_Group) {
			sca = getVector(scaleD.getGroup(), ok);

			if (!ok)
				PR_LOG(L_WARNING) << "Couldn't set scale for entity " << entity->name() << std::endl;
		}

		if (!ok) {
			return;
		} else {
			rot.normalize();

			VirtualEntity::Transform trans;
			trans.fromPositionOrientationScale(pos, rot, sca);
			trans.makeAffine();

			entity->setTransform(trans);
		}
	}
}

void SceneLoader::addEntity(const DL::DataGroup& group,
							const std::shared_ptr<PR::VirtualEntity>& parent, Environment* env)
{
	auto manag		= env->entityManager();
	const uint32 id = manag->nextID();

	DL::Data nameD		= group.getFromKey("name");
	DL::Data typeD		= group.getFromKey("type");
	DL::Data localAreaD = group.getFromKey("local_area");

	populateObjectRegistry(RG_ENTITY, id, group, env);

	std::string name;
	if (nameD.type() == DL::Data::T_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";

	std::string type;
	if (typeD.type() != DL::Data::T_String) {
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

	setupVirtualEntity(group, entity, env);

	if (parent) {
		entity->setTransform(parent->transform() * entity->transform());
	}

	if (localAreaD.type() == DL::Data::T_Bool) {
		if (localAreaD.getBool())
			entity->setFlags(entity->flags() | EF_LocalArea);
		else
			entity->setFlags(entity->flags() & ~(uint8)EF_LocalArea);
	}

	// Add to scene
	manag->addObject(entity);

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		if (group.at(i).type() == DL::Data::T_Group) {
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
	if (nameD.type() == DL::Data::T_String)
		name = nameD.getString();
	else
		name = "UNKNOWN";

	std::string type;
	if (typeD.type() != DL::Data::T_String) {
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

	setupVirtualEntity(group, camera, env);

	manag->addObject(camera);
}

void SceneLoader::addLight(const DL::DataGroup& group, Environment* env)
{
	auto manag		= env->infiniteLightManager();
	const uint32 id = manag->nextID();

	populateObjectRegistry(RG_INFINITELIGHT, id, group, env);

	DL::Data typeD = group.getFromKey("type");

	std::string type;

	if (typeD.type() == DL::Data::T_String) {
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

	setupVirtualEntity(group, light, env);

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
	if (nameD.type() == DL::Data::T_String) {
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
	if (typeD.type() == DL::Data::T_String) {
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

	if (nameD.type() == DL::Data::T_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No material name set" << std::endl;
		return;
	}

	if (env->hasMaterial(name)) {
		PR_LOG(L_ERROR) << "Material name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::Data::T_String) {
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

	if (shadeableD.type() == DL::Data::T_Bool)
		mat->enableShading(shadeableD.getBool());

	if (shadowD.type() == DL::Data::T_Bool)
		mat->enableShadow(shadowD.getBool());

	if (selfShadowD.type() == DL::Data::T_Bool)
		mat->enableSelfShadow(selfShadowD.getBool());

	if (cameraVisibleD.type() == DL::Data::T_Bool)
		mat->enableCameraVisibility(cameraVisibleD.getBool());

	env->addMaterial(name, mat);
	manag->addObject(mat);
}

void SceneLoader::addTexture(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD = group.getFromKey("name");

	std::string name;

	if (nameD.type() == DL::Data::T_String) {
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

	if (nameD.type() == DL::Data::T_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No mesh name set" << std::endl;
		return;
	}

	if (env->hasMesh(name)) {
		PR_LOG(L_ERROR) << "Mesh name already set" << std::endl;
		return;
	}

	if (typeD.type() == DL::Data::T_String) {
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

	TriMeshInlineParser parser;
	auto mesh = parser.parse(env, group);

	if (!mesh) {
		PR_LOG(L_ERROR) << "Mesh " << name << " couldn't be load. Error in " << typeD.getString() << " type parser." << std::endl;
		return;
	}

	PR_ASSERT(mesh, "After here it shouldn't be null");

	boost::filesystem::path path = env->workingDir();
	path /= (name+".cnt");

	mesh->build(path.generic_wstring());
	env->addMesh(name, mesh);
}

void SceneLoader::addSpectrum(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD = group.getFromKey("name");
	DL::Data dataD = group.getFromKey("data");

	std::string name;
	if (nameD.type() == DL::Data::T_String) {
		name = nameD.getString();
	} else {
		PR_LOG(L_ERROR) << "Couldn't get name for spectral entry." << std::endl;
		return;
	}

	if (env->hasSpectrum(name)) {
		PR_LOG(L_ERROR) << "Spectrum name already set" << std::endl;
		return;
	}

	Spectrum spec(env->spectrumDescriptor());
	if (dataD.type() == DL::Data::T_Group) {
		DL::DataGroup grp = dataD.getGroup();
		if (grp.isArray()) {
			for (size_t i = 0; i < grp.anonymousCount() && i < spec.samples(); ++i) {
				if (grp.at(i).isNumber())
					spec.setValue(i, grp.at(i).getNumber());
				else
					PR_LOG(L_WARNING) << "Couldn't set spectrum entry at index " << i << std::endl;
			}
		} else if (grp.id() == "field") {
			DL::Data defaultD = grp.getFromKey("default");

			if (defaultD.isNumber()) {
				for (uint32 i = 0; i < spec.samples(); ++i) {
					spec.setValue(i, defaultD.getNumber());
				}
			}

			for (uint32 i = 0;
				 i < grp.anonymousCount() && i < spec.samples();
				 ++i) {
				DL::Data fieldD = grp.at(i);

				if (fieldD.isNumber())
					spec.setValue(i, fieldD.getNumber());
			}
		} else if (grp.id() == "rgb") {
			if (grp.anonymousCount() == 3
				&& grp.isAllAnonymousNumber()) {
				RGBConverter::toSpec(spec,
									 grp.at(0).getNumber(),
									 grp.at(1).getNumber(),
									 grp.at(2).getNumber());
			}
		} else if (grp.id() == "xyz") {
			if (grp.anonymousCount() == 3
				&& grp.isAllAnonymousNumber()) {
				XYZConverter::toSpec(spec,
									 grp.at(0).getNumber(),
									 grp.at(1).getNumber(),
									 grp.at(2).getNumber());
			}
		} else if (grp.id() == "temperature" || grp.id() == "blackbody") { // Luminance
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.weightPhotometric();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_raw" || grp.id() == "blackbody_raw") { // Radiance
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_norm" || grp.id() == "blackbody_norm") { // Luminance Norm
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.weightPhotometric();
				spec.normalize();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_raw_norm" || grp.id() == "blackbody_raw_norm") { // Radiance Norm
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.normalize();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		}
	}

	//PR_LOG(L_INFO) << spec << std::endl;

	env->addSpectrum(name, spec);
}

void SceneLoader::addSubGraph(const DL::DataGroup& group, Environment* env)
{
	DL::Data nameD		= group.getFromKey("name");
	DL::Data overridesD = group.getFromKey("overrides");
	DL::Data loaderD	= group.getFromKey("loader");
	DL::Data fileD		= group.getFromKey("file");

	std::string name;
	if (nameD.type() == DL::Data::T_String)
		name = nameD.getString();

	std::map<std::string, std::string> overrides;
	if (overridesD.type() == DL::Data::T_String)
		overrides[""] = overridesD.getString();

	std::string file;
	if (fileD.type() == DL::Data::T_String) {
		file = fileD.getString();
	} else {
		PR_LOG(L_ERROR) << "Couldn't get file for subgraph entry." << std::endl;
		return;
	}

	std::string loader;
	if (loaderD.type() == DL::Data::T_String) {
		loader = loaderD.getString();
	} else {
		PR_LOG(L_WARNING) << "No valid loader set. Assuming 'obj'." << std::endl;
		loader = "obj";
	}

	if (loader == "obj") {
		DL::Data flipNormalD = group.getFromKey("flipNormal");

		WavefrontLoader loader(overrides);

		if (flipNormalD.type() == DL::Data::T_Bool)
			loader.flipNormal(flipNormalD.getBool());

		loader.load(file, env);
	} else {
		PR_LOG(L_ERROR) << "Unknown " << loader << " loader." << std::endl;
	}
}

Eigen::Matrix4f SceneLoader::getMatrix(const DL::DataGroup& grp, bool& ok)
{
	ok = false;
	if (grp.anonymousCount() == 16) {
		ok = true;
		for (int i = 0; i < 16; ++i) {
			if (!grp.at(i).isNumber()) {
				ok = false;
				break;
			}
		}

		if (ok) {
			Eigen::Matrix4f m;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					m(i, j) = grp.at(i * 4 + j).getNumber();
				}
			}

			return m;
		}
	}

	return Eigen::Matrix4f::Identity();
}

Vector3f SceneLoader::getVector(const DL::DataGroup& arr, bool& ok)
{
	Vector3f res(0, 0, 0);

	if (arr.anonymousCount() == 2) {
		if (arr.at(0).isNumber() && arr.at(1).isNumber()) {
			res = Vector3f(arr.at(0).getNumber(),
						   arr.at(1).getNumber(),
						   0);

			ok = true;
		} else {
			ok = false;
		}
	} else if (arr.anonymousCount() == 3) {
		if (arr.at(0).isNumber() && arr.at(1).isNumber() && arr.at(2).isNumber()) {
			res = Vector3f(arr.at(0).getNumber(),
						   arr.at(1).getNumber(),
						   arr.at(2).getNumber());

			ok = true;
		} else {
			ok = false;
		}
	} else {
		ok = false;
	}

	return res;
}

Eigen::Quaternionf SceneLoader::getRotation(const DL::Data& data, bool& ok)
{
	if (data.type() == DL::Data::T_Group) {
		DL::DataGroup grp = data.getGroup();
		if (grp.isArray() && grp.anonymousCount() == 4) {
			if (grp.at(0).isNumber() && grp.at(1).isNumber() && grp.at(2).isNumber() && grp.at(3).isNumber()) {
				ok = true;
				return Eigen::Quaternionf(grp.at(0).getNumber(),
										  grp.at(1).getNumber(),
										  grp.at(2).getNumber(),
										  grp.at(3).getNumber());
			} else {
				ok = false;
			}
		} else if (grp.id() == "euler" && grp.anonymousCount() == 3 && grp.at(0).isNumber() && grp.at(1).isNumber() && grp.at(2).isNumber()) {
			float x = grp.at(0).getNumber() * PR_PI / 180;
			float y = grp.at(1).getNumber() * PR_PI / 180;
			float z = grp.at(2).getNumber() * PR_PI / 180;

			Eigen::AngleAxisf ax(x, Vector3f::UnitX());
			Eigen::AngleAxisf ay(y, Vector3f::UnitY());
			Eigen::AngleAxisf az(z, Vector3f::UnitZ());

			ok = true;
			return az * ay * ax;
		}
	}

	return Eigen::Quaternionf::Identity();
}

std::shared_ptr<FloatSpectralShadingSocket> SceneLoader::getSpectralOutput(Environment* env, const DL::Data& dataD, bool allowScalar)
{
	if (allowScalar && dataD.isNumber()) {
		return std::make_shared<ConstSpectralShadingSocket>(
			Spectrum(env->spectrumDescriptor(), dataD.getNumber()));
	} else if (dataD.type() == DL::Data::T_String) {
		if (env->hasSpectrum(dataD.getString()))
			return std::make_shared<ConstSpectralShadingSocket>(env->getSpectrum(dataD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find spectrum " << dataD.getString() << " for material" << std::endl;
	} else if (dataD.type() == DL::Data::T_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::Data::T_String) {
				if (env->isShadingSocket<FloatSpectralShadingSocket>(nameD.getString()))
					return env->getShadingSocket<FloatSpectralShadingSocket>(nameD.getString());
				else
					PR_LOG(L_WARNING) << "Unknown spectral texture " << nameD.getString() << "." << std::endl;
			}
		} else {
			PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
}

std::shared_ptr<FloatScalarShadingSocket> SceneLoader::getScalarOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.isNumber()) {
		return std::make_shared<ConstScalarShadingSocket>(dataD.getNumber());
	} else if (dataD.type() == DL::Data::T_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::Data::T_String) {
				if (env->isShadingSocket<FloatScalarShadingSocket>(nameD.getString()))
					return env->getShadingSocket<FloatScalarShadingSocket>(nameD.getString());
				else
					PR_LOG(L_WARNING) << "Unknown scalar texture " << nameD.getString() << "." << std::endl;
			}
		} else {
			PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
}

std::shared_ptr<FloatVectorShadingSocket> SceneLoader::getVectorOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.type() == DL::Data::T_Group) {
		if (dataD.getGroup().isArray()) {
			bool ok;
			const auto vec = getVector(dataD.getGroup(), ok);

			if (ok) {
				return std::make_shared<ConstVectorShadingSocket>(vec);
			} else {
				PR_LOG(L_WARNING) << "Invalid vector entry." << std::endl;
			}
		} else {
			std::string name = dataD.getGroup().id();

			if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
				DL::Data nameD = dataD.getGroup().at(0);
				if (nameD.type() == DL::Data::T_String) {
					if (env->isShadingSocket<FloatVectorShadingSocket>(nameD.getString()))
						return env->getShadingSocket<FloatVectorShadingSocket>(nameD.getString());
					else
						PR_LOG(L_WARNING) << "Unknown vector texture " << nameD.getString() << std::endl;
				}
			} else {
				PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
			}
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
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
	case DL::Data::T_Integer:
		varAddReg<int64>(reg, regGroup, uuid, hasID, key, value.getInt());
		break;
	case DL::Data::T_Float:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getFloat());
		break;
	case DL::Data::T_Bool:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getBool());
		break;
	case DL::Data::T_String:
		varAddReg(reg, regGroup, uuid, hasID, key, value.getString());
		break;
	case DL::Data::T_Group: {
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
				if (entry.type() != DL::Data::T_String) {
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
			if (grp.anonymousCount() == 1 && grp.at(0).type() == DL::Data::T_String) {
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

		if (entry.type() != DL::Data::T_None)
			addRegistryEntry(regGroup, id, true, entry.key(), entry, env);
	}
}
} // namespace PR
