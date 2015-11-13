#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/BoundaryEntity.h"
#include "entity/SphereEntity.h"
#include "entity/MeshEntity.h"

#include "scene/Camera.h"

#include "material/DiffuseMaterial.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

#include <fstream>

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
					env->setCamera((Camera*)env->scene()->getEntity(cameraD->getString(), "camera"));
				}

				return env;
			}
		}
	}

	void SceneLoader::addEntity(DL::DataGroup* group, PR::Entity* parent, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* typeD = group->getFromKey("type");
		DL::Data* posD = group->getFromKey("position");
		DL::Data* rotD = group->getFromKey("rotation");
		DL::Data* scaleD = group->getFromKey("scale");
		DL::Data* materialD = group->getFromKey("material");

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
		if (!typeD || typeD->isType() != DL::Data::T_String || 
			(typeD->getString() != "null" && typeD->getString() != "sphere" &&
				typeD->getString() != "boundary" && typeD->getString() != "camera" &&
				typeD->getString() != "mesh"))
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. No valid type given.", name.c_str());
			return;
		}
		else if (typeD->getString() == "sphere")
		{
			DL::Data* radiusD = group->getFromKey("radius");

			float r = 1;
			if (radiusD && radiusD->isNumber())
			{
				r = radiusD->getFloatConverted();
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has no radius. Assuming 1.", name.c_str());
			}

			SphereEntity* sphere = new SphereEntity(name, r, parent);

			if (materialD && materialD->isType() == DL::Data::T_String)
			{
				if (env->hasMaterial(materialD->getString()))
				{
					sphere->setMaterial(env->getMaterial(materialD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
				}
			}

			entity = sphere;
		}
		else if (typeD->getString() == "boundary")
		{
			DL::Data* sizeD = group->getFromKey("size");

			PM::vec3 size = PM::pm_Set(1, 1, 1, 1);

			if (sizeD && (sizeD->isType() == DL::Data::T_Array || sizeD->isNumber()))
			{
				if (sizeD->isType() == DL::Data::T_Array)
				{
					DL::DataArray* arr = sizeD->getArray();

					bool ok;
					size = getVector(arr, ok);

					if (!ok)
					{
						size = PM::pm_Set(1, 1, 1, 1);
						PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid size. Assuming unit cube.", name.c_str());
					}
				}
				else
				{
					size = PM::pm_Set(sizeD->getFloatConverted(),
						sizeD->getFloatConverted(),
						sizeD->getFloatConverted(),
						1);
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has no size. Assuming unit cube.", name.c_str());
			}

			BoundaryEntity* bnd = new BoundaryEntity(name,
				BoundingBox(PM::pm_GetX(size), PM::pm_GetY(size), PM::pm_GetZ(size)),
				parent);

			if (materialD && materialD->isType() == DL::Data::T_String)
			{
				if (env->hasMaterial(materialD->getString()))
				{
					bnd->setMaterial(env->getMaterial(materialD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
				}
			}

			entity = bnd;
		}
		else if (typeD->getString() == "camera")
		{
			DL::Data* fovHD = group->getFromKey("fovH");
			DL::Data* fovVD = group->getFromKey("fovV");
			DL::Data* lensDistD = group->getFromKey("lensDistance");

			float fovH = 60;
			float fovV = 45;
			float lensDist = 0.1f;

			if (fovHD && fovHD->isNumber())
			{
				fovH = fovHD->getFloatConverted();
			}

			if (fovVD && fovVD->isNumber())
			{
				fovV = fovVD->getFloatConverted();
			}
			
			if (lensDistD && lensDistD->isNumber())
			{
				lensDist = lensDistD->getFloatConverted();
			}

			Camera* camera = new Camera(name, parent);
			camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV), lensDist);

			entity = camera;
		}
		else if (typeD->getString() == "mesh")
		{
			DL::Data* meshD = group->getFromKey("mesh");

			MeshEntity* me = new MeshEntity(name, parent);

			if (meshD && meshD->isType() == DL::Data::T_String)
			{
				if (env->hasMesh(meshD->getString()))
				{
					me->setMesh(env->getMesh(meshD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find mesh %s.", meshD->getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid mesh entry found.");
			}

			if (materialD && materialD->isType() == DL::Data::T_String)
			{
				if (env->hasMaterial(materialD->getString()))
				{
					me->setMaterial(env->getMaterial(materialD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
				}
			}

			entity = me;
		}
		else if (typeD->getString() == "null")
		{
			entity = new Entity(name, parent);
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
		if (rotD && rotD->isType() == DL::Data::T_Array)
		{
			bool ok;
			PM::quat rot = getVector(rotD->getArray(), ok);

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

		// Add to scene
		if (typeD->getString() == "null" || typeD->getString() == "camera")
		{
			env->scene()->addEntity(entity);
		}
		else
		{
			env->scene()->addEntity((GeometryEntity*)entity);
		}

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
			DL::Data* spectrumD = group->getFromKey("spectrum");
			DL::Data* emissionD = group->getFromKey("emission");
			DL::Data* roughnessD = group->getFromKey("roughness");
			DL::Data* shadingD = group->getFromKey("enableShading");

			Spectrum spec;
			if (spectrumD && spectrumD->isType() == DL::Data::T_String)
			{
				if (env->hasSpectrum(spectrumD->getString()))
				{
					spec = env->getSpectrum(spectrumD->getString());
				}
				else
				{
					PR_LOGGER.logf(L_Error, M_Scene, "Couldn't find spectrum '%s' for material",
						spectrumD->getString().c_str());
					return;
				}
			}
			else
			{
				PR_LOGGER.logf(L_Error, M_Scene, "No spectrum given for material");
				return;
			}

			DiffuseMaterial* diff = new DiffuseMaterial(spec);

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

			mat = diff;
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
		
		if (dataD && dataD->isType() == DL::Data::T_Array)
		{
			DL::DataArray* arr = dataD->getArray();
			for (size_t i = 0; i < arr->size() && i < Spectrum::SAMPLING_COUNT; ++i)
			{
				if (arr->at(i)->isNumber())
				{
					spec.setValue(i, arr->at(i)->getFloatConverted());
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set spectral entry at index %i.", i);
				}
			}
		}

		env->addSpectrum(name, spec);
	}

	void SceneLoader::addMesh(DL::DataGroup* group, Environment* env)
	{

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
}