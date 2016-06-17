#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/Entity.h"
#include "entity/BoundaryEntity.h"// For debug

#include "parser/entity/BoundaryParser.h"
#include "parser/entity/CameraParser.h"
#include "parser/entity/MeshParser.h"
#include "parser/entity/PlaneParser.h"
#include "parser/entity/SphereParser.h"

#include "parser/material/BRDFMaterialParser.h"
#include "parser/material/DebugBoundingBoxMaterialParser.h"
#include "parser/material/DiffuseMaterialParser.h"
#include "parser/material/GlassMaterialParser.h"
#include "parser/material/GridMaterialParser.h"
#include "parser/material/MirrorMaterialParser.h"

#include "texture/ConstData1D.h"
#include "texture/ConstData2D.h"
#include "texture/ConstTexture1D.h"
#include "texture/ConstTexture2D.h"
#include "texture/MapData1D.h"
#include "texture/MapData2D.h"
#include "texture/RGBTexture1D.h"
#include "texture/RGBTexture2D.h"
#include "texture/SpecTexture1D.h"
#include "texture/SpecTexture2D.h"

#include "geometry/IMesh.h"
#include "loader/WavefrontLoader.h"

#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

#include "material/Material.h"

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
				DL::Data* renderWidthD = top->getFromKey("renderWidth");
				DL::Data* renderHeightD = top->getFromKey("renderHeight");
				DL::Data* cropD = top->getFromKey("crop");

				if (!nameD || nameD->isType() != DL::Data::T_String)
				{
					env = new Environment("UNKNOWN");
				}
				else
				{
					env = new Environment(nameD->getString());
				}

				if (renderWidthD && renderWidthD->isType() == DL::Data::T_Integer)
				{
					env->setRenderWidth(renderWidthD->getInt());
				}
				else
				{
					env->setRenderWidth(1920);
				}

				if (renderHeightD && renderHeightD->isType() == DL::Data::T_Integer)
				{
					env->setRenderHeight(renderHeightD->getInt());
				}
				else
				{
					env->setRenderHeight(1080);
				}

				if (cropD && cropD->isType() == DL::Data::T_Array)
				{
					DL::DataArray* arr = cropD->getArray();
					if (arr->size() == 4)
					{
						if (arr->isAllNumber())
						{
							env->setCrop(arr->at(0)->getFloatConverted(), arr->at(1)->getFloatConverted(),
								arr->at(2)->getFloatConverted(), arr->at(3)->getFloatConverted());
						}
					}
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
						else if (entry->id() == "graph")
						{
							addSubGraph(entry, env);
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

				DL::Data* backgroundD = top->getFromKey("background");
				if (backgroundD && backgroundD->isType() == DL::Data::T_String)
				{
					if (env->hasMaterial(backgroundD->getString()))
					{
						env->setBackgroundMaterial(env->getMaterial(backgroundD->getString()));
					}
					else
					{
						PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", backgroundD->getString().c_str());
					}
				}
				else
				{
					PR_LOGGER.log(L_Warning, M_Scene, "Invalid background entry.");
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
		const EntityParser& Parser;
	} EntityParserEntries[] =
	{
		{ "boundary", BoundaryParser() },
		{ "box", BoundaryParser() },
		{ "camera", CameraParser() },
		{ "mesh", MeshParser() },
		{ "plane", PlaneParser() },
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
			const EntityParser* parser = nullptr;
			for (int i = 0; EntityParserEntries[i].Name; ++i)
			{
				if (typeD->getString() == EntityParserEntries[i].Name)
				{
					parser = &EntityParserEntries[i].Parser;
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
		if (scaleD && scaleD->isNumber())
		{
			entity->setScale(scaleD->getFloatConverted());
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

	struct
	{
		const char* Name;
		const MaterialParser& Parser;
	} MaterialParserEntries[] =
	{
		{ "standard", BRDFMaterialParser() },
		{ "light", DiffuseMaterialParser() },

		{ "diffuse", DiffuseMaterialParser() },
		{ "brdf", BRDFMaterialParser() },

		{ "debugBoundingBox", DebugBoundingBoxMaterialParser() },

		{ "grid", GridMaterialParser() },
		{ "glass", GlassMaterialParser() },
		{ "mirror", MirrorMaterialParser() },

		{ nullptr, BRDFMaterialParser() },//Just for the end
	};
	void SceneLoader::addMaterial(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* typeD = group->getFromKey("type");

		DL::Data* selfShadowD = group->getFromKey("selfShadow");
		DL::Data* cameraVisibleD = group->getFromKey("cameraVisible");
		DL::Data* shadeableD = group->getFromKey("shadeable");
		DL::Data* emissionD = group->getFromKey("emission");

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
		const MaterialParser* parser = nullptr;
		for (int i = 0; MaterialParserEntries[i].Name; ++i)
		{
			if (typeD->getString() == MaterialParserEntries[i].Name)
			{
				parser = &MaterialParserEntries[i].Parser;
				break;
			}
		}

		if (parser)
		{
			mat = parser->parse(this, env, typeD->getString(), group);

			if (!mat)
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Material %s couldn't be load. Error in '%s' type parser.",
					name.c_str(), typeD->getString().c_str());
				return;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Material %s couldn't be load. Unknown type given.", name.c_str());
			return;
		}

		PR_ASSERT(mat);// After here it shouldn't be null

		mat->setEmission(getTexture2D(env, emissionD));

		if (shadeableD && shadeableD->isType() == DL::Data::T_Bool)
		{
			mat->enableShading(shadeableD->getBool());
		}

		if (selfShadowD && selfShadowD->isType() == DL::Data::T_Bool)
		{
			mat->enableSelfShadow(selfShadowD->getBool());
		}

		if (cameraVisibleD && cameraVisibleD->isType() == DL::Data::T_Bool)
		{
			mat->enableCameraVisibility(cameraVisibleD->getBool());
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
				else if (grp->id() == "temperature" || grp->id() == "blackbody")
				{
					if (grp->unnamedCount() == 1 &&
						grp->at(0)->isNumber())
					{
						spec = Spectrum::fromBlackbody(PM::pm_MaxT(0.0f, grp->at(0)->getFloatConverted()));
						spec.setEmissive(true);
					}
				}
				else if (grp->id() == "temperature_norm" || grp->id() == "blackbody_norm")
				{
					if (grp->unnamedCount() >= 1 &&
						grp->at(0)->isNumber())
					{
						spec = Spectrum::fromBlackbodyNorm(PM::pm_MaxT(0.0f, grp->at(0)->getFloatConverted()));
						spec.setEmissive(true);
					}

					if (grp->unnamedCount() >= 2 &&
						grp->at(1)->isNumber())
					{
						spec *= grp->at(1)->getFloatConverted();
					}
				}
			}
		}

		env->addSpectrum(name, spec);
	}

	void SceneLoader::addSubGraph(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* overridesD = group->getFromKey("overrides");
		DL::Data* loaderD = group->getFromKey("loader");
		DL::Data* fileD = group->getFromKey("file");

		std::string name;
		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}

		std::map<std::string, std::string> overrides;
		if (overridesD && overridesD->isType() == DL::Data::T_String)
		{
			overrides[""] = overridesD->getString();
		}
		/*else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get names for subgraph entry.");
			return;
		}*/
		
		std::string file;
		if (fileD && fileD->isType() == DL::Data::T_String)
		{
			file = fileD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get file for subgraph entry.");
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
			DL::Data* flipNormalD = group->getFromKey("flipNormal");
			
			WavefrontLoader loader(overrides);

			if (flipNormalD && flipNormalD->isType() == DL::Data::T_Bool)
			{
				loader.flipNormal(flipNormalD->getBool());
			}

			loader.load(file, env);
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

	Texture2D* SceneLoader::getTexture2D(Environment* env, DL::Data* dataD) const
	{
		if (!dataD)
			return nullptr;

		if (dataD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(dataD->getString()))
			{
				ConstTexture2D* tex = new ConstTexture2D(env->getSpectrum(dataD->getString()));
				env->addTexture2D(tex);
				return tex;
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					dataD->getString().c_str());
			}
		}
		else if (dataD->isType() == DL::Data::T_Group)
		{
			std::string name = dataD->getGroup()->id();

			if (name == "file")
			{
				if (dataD->getGroup()->unnamedCount() == 1)
				{
					DL::Data* filenameD = dataD->getGroup()->at(0);
					if (filenameD->isType() == DL::Data::T_String)
					{
						if (!env->hasTexture2D(filenameD->getString()))
						{
							Texture2D* tex = mImageLoader.load(filenameD->getString());
							if (tex)
								env->addTexture2D(filenameD->getString(), tex);
							return tex;
						}
						else
						{
							return env->getTexture2D(filenameD->getString());
						}
					}
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
			}
		}

		return nullptr;
	}

	Data1D* SceneLoader::getData1D(Environment* env, DL::Data* dataD) const
	{
		if (!dataD)
			return nullptr;

		if (dataD->isNumber())
		{
			ConstData1D* data = new ConstData1D(dataD->getFloatConverted());
			env->addData1D(data);
			return data;
		}
		else if (dataD->isType() == DL::Data::T_Group)
		{
			std::string name = dataD->getGroup()->id();

			if (name == "file")
			{
				//TODO
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}

		return nullptr;
	}

	Data2D* SceneLoader::getData2D(Environment* env, DL::Data* dataD) const
	{
		if (!dataD)
			return nullptr;

		if (dataD->isNumber())
		{
			ConstData2D* data = new ConstData2D(dataD->getFloatConverted());
			env->addData2D(data);
			return data;
		}
		else if (dataD->isType() == DL::Data::T_Group)
		{
			std::string name = dataD->getGroup()->id();

			if (name == "file")
			{
				// TODO:
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}

		return nullptr;
	}
}