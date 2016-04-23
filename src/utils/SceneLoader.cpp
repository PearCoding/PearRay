#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/Entity.h"
#include "entity/BoundaryEntity.h"// For debug

#include "parser/BoundaryParser.h"
#include "parser/CameraParser.h"
#include "parser/GridParser.h"
#include "parser/LightParser.h"
#include "parser/MeshParser.h"
#include "parser/PlaneParser.h"
#include "parser/SphereParser.h"

#include "material/DiffuseMaterial.h"
#include "material/DebugMaterial.h"
#include "material/DebugBoundingBoxMaterial.h"

#include "geometry/Mesh.h"
#include "loader/WavefrontLoader.h"

#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

#include <fstream>
#include <sstream>

using namespace PR;
namespace PRU
{
	SceneLoader::SceneLoader()
	{
	}

	SceneLoader::~SceneLoader()
	{
	}

	Environment* SceneLoader::loadFromFile(const std::string& path)
	{
		std::ifstream stream(path);
		std::string str((std::istreambuf_iterator<char>(stream)),
			std::istreambuf_iterator<char>());

		return load(str);
	}

	Environment* SceneLoader::load(const std::string& source)
	{
		DL::SourceLogger logger;
		DL::DataLisp dataLisp(&logger);
		DL::DataContainer container;

		dataLisp.parse(source);
		dataLisp.build(&container);

		std::list<DL::DataGroup*> entries = container.getTopGroups();

		if (entries.empty())
		{
			PR_LOGGER.log(L_Error, M_Scene, "DataLisp file does not contain valid entries");
			return nullptr;
		}
		else
		{
			DL::DataGroup* top = entries.front();

			if (top->id() != "scene")
			{
				PR_LOGGER.log(L_Error, M_Scene, "DataLisp file does not contain valid top entry");
				return nullptr;
			}
			else
			{
				Environment* env;
				DL::Data* nameD = top->getFromKey("name");
				if (!nameD || nameD->isType() != DL::Data::T_String)
				{
					env = new Environment("UNKNOWN");
				}
				else
				{
					env = new Environment(nameD->getString());
				}

				// First independent information
				for (size_t i = 0; i < top->unnamedCount(); ++i)
				{
					DL::Data* dataD = top->at(i);

					if (dataD && dataD->isType() == DL::Data::T_Group)
					{
						DL::DataGroup* entry = dataD->getGroup();

						if (entry->id() == "spectrum")
						{
							addSpectrum(entry, env);
						}
						else if (entry->id() == "texture")
						{
							//addTexture(entry, env);
						}
						else if (entry->id() == "mesh")
						{
							addMesh(entry, env);
						}
					}
				}

				// Now semi-dependent information
				for (size_t i = 0; i < top->unnamedCount(); ++i)
				{
					DL::Data* dataD = top->at(i);

					if (dataD && dataD->isType() == DL::Data::T_Group)
					{
						DL::DataGroup* entry = dataD->getGroup();

						if (entry->id() == "material")
						{
							addMaterial(entry, env);
						}
					}
				}

				// Now entities.
				for (size_t i = 0; i < top->unnamedCount(); ++i)
				{
					DL::Data* dataD = top->at(i);

					if (dataD && dataD->isType() == DL::Data::T_Group)
					{
						DL::DataGroup* entry = dataD->getGroup();

						if (entry->id() == "entity")
						{
							addEntity(entry, nullptr, env);
						}
					}
				}

				DL::Data* cameraD = top->getFromKey("camera");
				if (cameraD && cameraD->isType() == DL::Data::T_String)
				{
					Camera* cam = (Camera*)env->scene()->getEntity(cameraD->getString(), "orthographicCamera");
					if (!cam)
					{
						cam = (Camera*)env->scene()->getEntity(cameraD->getString(), "perspectiveCamera");
					}

					env->setCamera(cam);
				}

				return env;
			}
		}
	}

	struct
	{
		const char* Name;
		EntityParser& Parser;
	} ParserEntries[] =
	{
		{ "boundary", BoundaryParser() },
		{ "camera", CameraParser() },
		{ "grid", GridParser() },
		{ "mesh", MeshParser() },
		{ "plane", PlaneParser() },
		{ "pointLight", LightParser() },
		{ "sphere", SphereParser() },

		{ nullptr, BoundaryParser() },//Just for the end
	};

	void SceneLoader::addEntity(DL::DataGroup* group, PR::Entity* parent, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* typeD = group->getFromKey("type");
		DL::Data* posD = group->getFromKey("position");
		DL::Data* rotD = group->getFromKey("rotation");
		DL::Data* scaleD = group->getFromKey("scale");
		DL::Data* debugD = group->getFromKey("debug");
		DL::Data* materialDebugBoundingBoxD = group->getFromKey("materialDebugBoundingBox");

		std::string name;
		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			name = "UNKNOWN";
		}

		Entity* entity = nullptr;
		if (!typeD || typeD->isType() != DL::Data::T_String)
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. No valid type given.", name.c_str());
			return;
		}
		else if (typeD->getString() == "null")
		{
			entity = new Entity(name, parent);
		}
		else
		{
			EntityParser* parser = nullptr;
			for (int i = 0; ParserEntries[i].Name; ++i)
			{
				if (typeD->getString() == ParserEntries[i].Name)
				{
					parser = &ParserEntries[i].Parser;
					break;
				}
			}

			if (parser)
			{
				entity = parser->parse(this, env, name, parent, typeD->getString(), group);

				if (!entity)
				{
					PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. Error in '%s' type parser.",
						name.c_str(), typeD->getString().c_str());
					return;
				}
			}
			else
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. Unknown type given.", name.c_str());
				return;
			}
		}

		PR_ASSERT(entity);// After here it shouldn't be null

		// Set position
		if (posD && posD->isType() == DL::Data::T_Array)
		{
			bool ok;
			entity->setPosition(getVector(posD->getArray(), ok));

			if (!ok)
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set position for entity %s.", name.c_str());
			}
		}

		// Set rotation
		if (rotD)
		{
			bool ok;
			PM::quat rot = getRotation(rotD, ok);

			if (!ok)
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set rotation for entity %s.", name.c_str());
			}
			else
			{
				entity->setRotation(rot);
			}
		}

		// Set scale
		if (scaleD && scaleD->isType() == DL::Data::T_Array)
		{
			bool ok;
			entity->setScale(getVector(scaleD->getArray(), ok));

			if (!ok)
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set scale for entity %s.", name.c_str());
			}
		}
		else if (scaleD && scaleD->isNumber())
		{
			entity->setScale(PM::pm_Set(
				scaleD->getFloatConverted(), scaleD->getFloatConverted(), scaleD->getFloatConverted(), 1));
		}

		// Debug
		if (debugD && debugD->isType() == DL::Data::T_Bool)
		{
			entity->enableDebug(debugD->getBool());

			if (typeD->getString() != "null" && typeD->getString() != "camera")
			{
				BoundaryEntity* bent = new BoundaryEntity("_debug_entity_",
					((RenderEntity*)entity)->localBoundingBox(), entity);

				if (materialDebugBoundingBoxD && materialDebugBoundingBoxD->isType() == DL::Data::T_String)
				{
					if (env->hasMaterial(materialDebugBoundingBoxD->getString()))
					{
						bent->setMaterial(env->getMaterial(materialDebugBoundingBoxD->getString()));
					}
					else
					{
						PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialDebugBoundingBoxD->getString().c_str());
					}
				}

				//bent->setScale(entity->scale());
				env->scene()->addEntity((RenderEntity*)bent);
			}
		}

		// Add to scene
		env->scene()->addEntity(entity);

		for (size_t i = 0; i < group->unnamedCount(); ++i)
		{
			if (group->at(i)->isType() == DL::Data::T_Group)
			{
				DL::DataGroup* child = group->at(i)->getGroup();

				if (child->id() == "entity")
				{
					addEntity(child, entity, env);
				}
			}
		}
	}

	void SceneLoader::addMaterial(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* typeD = group->getFromKey("type");

		std::string name;
		std::string type;

		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No material name set");
			return;
		}

		if (typeD && typeD->isType() == DL::Data::T_String)
		{
			type = typeD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No material type set");
			return;
		}

		Material* mat = nullptr;
		if (type == "standard")
		{
			DL::Data* reflectanceD = group->getFromKey("reflectance");
			DL::Data* emissionD = group->getFromKey("emission");
			DL::Data* roughnessD = group->getFromKey("roughness");
			DL::Data* shadingD = group->getFromKey("shading");
			DL::Data* lightD = group->getFromKey("light");
			DL::Data* selfShadowD = group->getFromKey("selfShadow");
			DL::Data* cameraVisibleD = group->getFromKey("cameraVisible");

			DiffuseMaterial* diff = new DiffuseMaterial;

			if (reflectanceD && reflectanceD->isType() == DL::Data::T_String)
			{
				if (env->hasSpectrum(reflectanceD->getString()))
				{
					diff->setReflectance(env->getSpectrum(reflectanceD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
						reflectanceD->getString().c_str());
				}
			}

			if (emissionD && emissionD->isType() == DL::Data::T_String)
			{
				if (env->hasSpectrum(emissionD->getString()))
				{
					diff->setEmission(env->getSpectrum(emissionD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
						emissionD->getString().c_str());
				}
			}

			if (roughnessD && roughnessD->isNumber())
			{
				diff->setRoughness(roughnessD->getFloatConverted());
			}

			if (shadingD && shadingD->isType() == DL::Data::T_Bool)
			{
				diff->enableShading(shadingD->getBool());
			}

			if (lightD && lightD->isType() == DL::Data::T_Bool)
			{
				diff->enableLight(lightD->getBool());
			}

			if (selfShadowD && selfShadowD->isType() == DL::Data::T_Bool)
			{
				diff->enableSelfShadow(selfShadowD->getBool());
			}

			if (cameraVisibleD && cameraVisibleD->isType() == DL::Data::T_Bool)
			{
				diff->enableCameraVisibility(cameraVisibleD->getBool());
			}

			mat = diff;
		}
		else if (type == "debug")
		{
			mat = new DebugMaterial();
		}
		else if (type == "debugBoundingBox")
		{
			DL::Data* colorD = group->getFromKey("color");
			DL::Data* densityD = group->getFromKey("density");

			DebugBoundingBoxMaterial* dbbm = new DebugBoundingBoxMaterial();

			if (colorD && colorD->isType() == DL::Data::T_String)
			{
				if (env->hasSpectrum(colorD->getString()))
				{
					dbbm->setColor(env->getSpectrum(colorD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
						colorD->getString().c_str());
				}
			}

			if (densityD && densityD->isNumber())
			{
				dbbm->setDensity(densityD->getFloatConverted());
			}

			mat = dbbm;
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Unknown material type '%s'", type.c_str());
			return;
		}

		env->addMaterial(name, mat);
	}

	void SceneLoader::addSpectrum(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* emissiveD = group->getFromKey("emissive");
		DL::Data* dataD = group->getFromKey("data");

		std::string name;
		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get name for spectral entry.");
			return;
		}

		Spectrum spec;
		
		if (emissiveD && emissiveD->isType() == DL::Data::T_Bool)
		{
			spec.setEmissive(emissiveD->getBool());
		}

		if (dataD)
		{
			if (dataD->isType() == DL::Data::T_Array)
			{
				DL::DataArray* arr = dataD->getArray();
				for (size_t i = 0; i < arr->size() && i < Spectrum::SAMPLING_COUNT; ++i)
				{
					if (arr->at(i)->isNumber())
					{
						spec.setValue((uint32)i, arr->at(i)->getFloatConverted());
					}
					else
					{
						PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set spectrum entry at index %i.", i);
					}
				}
			}
			else if (dataD->isType() == DL::Data::T_Group)
			{
				DL::DataGroup* grp = dataD->getGroup();

				if (grp->id() == "field")
				{
					DL::Data* defaultD = grp->getFromKey("default");

					if (defaultD && defaultD->isNumber())
					{
						for (uint32 i = 0; i <= PR::Spectrum::SAMPLING_COUNT; ++i)
						{
							spec.setValue(i, defaultD->getFloatConverted());
						}
					}

					for (uint32 i = 0; i <= PR::Spectrum::SAMPLING_COUNT; ++i)
					{
						std::stringstream stream;
						stream << (i*PR::Spectrum::WAVELENGTH_STEP + PR::Spectrum::WAVELENGTH_START);

						DL::Data* fieldD = grp->getFromKey(stream.str());

						if (fieldD && fieldD->isNumber())
						{
							spec.setValue(i, fieldD->getFloatConverted());
						}
						/*else
						{
							PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set spectrum entry at %s.",
								stream.str().c_str());
						}*/
					}
				}
				else if (grp->id() == "rgb")
				{
					if (grp->unnamedCount() == 3 &&
						grp->at(0)->isNumber() &&
						grp->at(1)->isNumber() &&
						grp->at(2)->isNumber())
					{
						bool em = spec.isEmissive();

						spec = RGBConverter::toSpec(grp->at(0)->getFloatConverted(),
							grp->at(1)->getFloatConverted(),
							grp->at(2)->getFloatConverted());
						spec.setEmissive(em);
					}
				}
				else if (grp->id() == "xyz")
				{
					// TODO
				}
			}
		}

		env->addSpectrum(name, spec);
	}

	void SceneLoader::addMesh(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* loaderD = group->getFromKey("loader");
		DL::Data* fileD = group->getFromKey("file");

		std::string name;
		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get name for mesh entry.");
			return;
		}
		
		std::string file;
		if (fileD && fileD->isType() == DL::Data::T_String)
		{
			file = fileD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get file for mesh entry.");
			return;
		}

		std::string loader;
		if (loaderD && loaderD->isType() == DL::Data::T_String)
		{
			loader = loaderD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "No valid loader set. Assuming 'obj'.");
			loader = "obj";
		}

		if (loader == "obj")
		{
			Mesh* mesh = new Mesh;

			WavefrontLoader loader;
			loader.load(file, mesh);

			env->addMesh(name, mesh);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Unknown '%s' loader.", loader.c_str());
		}
	}

	PM::vec3 SceneLoader::getVector(DL::DataArray* arr, bool& ok) const
	{
		PR_ASSERT(arr);

		PM::vec3 res = PM::pm_Set(0, 0, 0, 1);

		if (arr->size() == 2)
		{
			if (arr->at(0)->isNumber() &&
				arr->at(1)->isNumber())
			{
				res = PM::pm_Set(arr->at(0)->getFloatConverted(),
					arr->at(1)->getFloatConverted(),
					0,
					1);

				ok = true;
			}
			else
			{
				ok = false;
			}
		}
		else if (arr->size() == 3)
		{
			if (arr->at(0)->isNumber() &&
				arr->at(1)->isNumber() &&
				arr->at(2)->isNumber())
			{
				res = PM::pm_Set(arr->at(0)->getFloatConverted(),
					arr->at(1)->getFloatConverted(),
					arr->at(2)->getFloatConverted(),
					1);

				ok = true;
			}
			else
			{
				ok = false;
			}
		}
		else if (arr->size() == 4)
		{
			if (arr->at(0)->isNumber() &&
				arr->at(1)->isNumber() &&
				arr->at(2)->isNumber() &&
				arr->at(3)->isNumber())
			{
				res = PM::pm_Set(arr->at(0)->getFloatConverted(),
					arr->at(1)->getFloatConverted(),
					arr->at(2)->getFloatConverted(),
					arr->at(3)->getFloatConverted());

				ok = true;
			}
			else
			{
				ok = false;
			}
		}
		else
		{
			ok = false;
		}

		return res;
	}
	
	PM::quat SceneLoader::getRotation(DL::Data* data, bool& ok) const
	{
		if (data->isType() == DL::Data::T_Array)
		{
			return PM::pm_Normalize4D(getVector(data->getArray(), ok));
		}
		else if (data->isType() == DL::Data::T_Group)
		{
			DL::DataGroup* grp = data->getGroup();
			if (grp->id() == "euler" && grp->unnamedCount() == 3 &&
				grp->at(0)->isNumber() && grp->at(1)->isNumber() && grp->at(2)->isNumber())
			{
				float x = PM::pm_DegToRad(grp->at(0)->getFloatConverted());
				float y = PM::pm_DegToRad(grp->at(1)->getFloatConverted());
				float z = PM::pm_DegToRad(grp->at(2)->getFloatConverted());

				ok = true;
				return PM::pm_Normalize4D(PM::pm_RotationQuatRollPitchYaw(x, y, z));
			}
		}

		return PM::pm_IdentityQuat();
	}
}