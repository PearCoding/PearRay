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

#include "parser/material/BlinnPhongMaterialParser.h"
#include "parser/material/DebugBoundingBoxMaterialParser.h"
#include "parser/material/DiffuseMaterialParser.h"
#include "parser/material/GlassMaterialParser.h"
#include "parser/material/GridMaterialParser.h"
#include "parser/material/MirrorMaterialParser.h"
#include "parser/material/OrenNayarMaterialParser.h"
#include "parser/material/WardMaterialParser.h"

#include "parser/mesh/TriMeshInlineParser.h"

#include "parser/light/EnvironmentLightParser.h"
#include "parser/light/DistantLightParser.h"

#include "parser/texture/TextureParser.h"

#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ConstVectorOutput.h"
#include "shader/ImageScalarOutput.h"
#include "shader/ImageSpectralOutput.h"
#include "shader/ImageVectorOutput.h"

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

		//PR_LOGGER.log(L_Info, M_Scene, dataLisp.dump());

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

				if (renderHeightD && renderHeightD->isType() == DL::Data::T_Integer)
				{
					env->setRenderHeight(renderHeightD->getInt());
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
							addTexture(entry, env);
						}
						else if (entry->id() == "graph")
						{
							addSubGraph(entry, env);
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

				// Now entities and lights
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
						else if(entry->id() == "light")
						{
							addLight(entry, env);
						}
					}
				}
				
				DL::Data* cameraD = top->getFromKey("camera");
				if (cameraD && cameraD->isType() == DL::Data::T_String)
				{
					Camera* cam = (Camera*)env->scene()->getEntity(cameraD->getString(), "standard_camera");
					env->setCamera(cam);
				}

				return env;
			}
		}
	}

	struct
	{
		const char* Name;
		const IEntityParser& Parser;
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
		DL::Data* localAreaD = group->getFromKey("local_area");

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
		else if (typeD->getString() == "null" || typeD->getString() == "empty")
		{
			entity = new Entity(name, parent);
		}
		else
		{
			const IEntityParser* parser = nullptr;
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
			PM::vec p = getVector(posD->getArray(), ok);

			if (!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set position for entity %s.", name.c_str());
			else
				entity->setPosition(p);
		}

		// Set rotation
		if (rotD)
		{
			bool ok;
			PM::quat rot = getRotation(rotD, ok);

			if (!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set rotation for entity %s.", name.c_str());
			else
				entity->setRotation(rot);
		}

		// Set scale
		if (scaleD && scaleD->isNumber())
		{
			float s = scaleD->getFloatConverted();
			entity->setScale(PM::pm_Set(s, s, s, 1));
		}
		else if(scaleD && scaleD->isType() == DL::Data::T_Array)
		{
			bool ok;
			PM::vec3 s = PM::pm_SetW(getVector(scaleD->getArray(), ok), 1);

			if(!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set scale for entity %s.", name.c_str());
			else
				entity->setScale(s);
		}

		// Debug
		if (debugD && debugD->isType() == DL::Data::T_Bool)
		{
			if(debugD->getBool())
				entity->setFlags(entity->flags() | EF_Debug);
			else
				entity->setFlags(entity->flags() & ~(uint8)EF_Debug);

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

		if (localAreaD && localAreaD->isType() == DL::Data::T_Bool)
		{
			if(localAreaD->getBool())
				entity->setFlags(entity->flags() | EF_LocalArea);
			else
				entity->setFlags(entity->flags() & ~(uint8)EF_LocalArea);
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
		const ILightParser& Parser;
	} LightParserEntries[] =
	{
		{ "env", EnvironmentLightParser() },
		{ "environment", EnvironmentLightParser() },
		{ "background", EnvironmentLightParser() },

		{ "distant", DistantLightParser() },
		{ "sun", DistantLightParser() },
		{ nullptr, EnvironmentLightParser() },//Just for the end
	};
	void SceneLoader::addLight(DL::DataGroup* group, Environment* env)
	{
		DL::Data* typeD = group->getFromKey("type");

		std::string type;

		if (typeD && typeD->isType() == DL::Data::T_String)
		{
			type = typeD->getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No light type set");
			return;
		}

		IInfiniteLight* light = nullptr;
		const ILightParser* parser = nullptr;
		for (int i = 0; LightParserEntries[i].Name; ++i)
		{
			if (typeD->getString() == LightParserEntries[i].Name)
			{
				parser = &LightParserEntries[i].Parser;
				break;
			}
		}

		if (parser)
		{
			light = parser->parse(this, env, group);

			if (!light)
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Light couldn't be load. Error in '%s' type parser.",
					typeD->getString().c_str());
				return;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Light couldn't be load. Unknown type given.");
			return;
		}

		PR_ASSERT(light);// After here it shouldn't be null
		env->scene()->addInfiniteLight(light);
	}

	struct
	{
		const char* Name;
		const IMaterialParser& Parser;
	} MaterialParserEntries[] =
	{
		{ "standard", WardMaterialParser() },
		{ "light", DiffuseMaterialParser() },

		{ "diffuse", DiffuseMaterialParser() },
		{ "orennayar", OrenNayarMaterialParser() },
		{ "blinnphong", BlinnPhongMaterialParser() },
		{ "ward", WardMaterialParser() },

		{ "debug_bounding_box", DebugBoundingBoxMaterialParser() },

		{ "grid", GridMaterialParser() },
		{ "glass", GlassMaterialParser() },
		{ "mirror", MirrorMaterialParser() },

		{ nullptr, DiffuseMaterialParser() },//Just for the end
	};
	void SceneLoader::addMaterial(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");
		DL::Data* typeD = group->getFromKey("type");

		DL::Data* shadowD = group->getFromKey("shadow");
		DL::Data* selfShadowD = group->getFromKey("self_shadow");
		DL::Data* cameraVisibleD = group->getFromKey("camera_visible");
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
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No material type set");
			return;
		}

		Material* mat = nullptr;
		const IMaterialParser* parser = nullptr;
		for (int i = 0; MaterialParserEntries[i].Name; ++i)
		{
			if (type == MaterialParserEntries[i].Name)
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

		mat->setEmission(getSpectralOutput(env, emissionD));

		if (shadeableD && shadeableD->isType() == DL::Data::T_Bool)
		{
			mat->enableShading(shadeableD->getBool());
		}

		if (shadowD && shadowD->isType() == DL::Data::T_Bool)
		{
			mat->enableShadow(shadowD->getBool());
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

	void SceneLoader::addTexture(DL::DataGroup* group, Environment* env)
	{
		DL::Data* nameD = group->getFromKey("name");

		std::string name;

		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No texture name set");
			return;
		}

		TextureParser parser;
		parser.parse(this, env, name, group);// Will be added to env here
	}

	struct
	{
		const char* Name;
		const IMeshInlineParser& Parser;
	} MeshInlineParserEntries[] =
	{
		{ "triangles", TriMeshInlineParser() },
		{ nullptr, TriMeshInlineParser() },//Just for the end
	};
	void SceneLoader::addMesh(DL::DataGroup* group, Environment* env)
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
			PR_LOGGER.logf(L_Error, M_Scene, "No mesh name set");
			return;
		}

		if (typeD && typeD->isType() == DL::Data::T_String)
		{
			type = typeD->getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No mesh type set");
			return;
		}

		IMesh* mesh = nullptr;
		const IMeshInlineParser* parser = nullptr;
		for (int i = 0; MeshInlineParserEntries[i].Name; ++i)
		{
			if (type == MeshInlineParserEntries[i].Name)
			{
				parser = &MeshInlineParserEntries[i].Parser;
				break;
			}
		}

		if (parser)
		{
			mesh = parser->parse(this, env, group);

			if (!mesh)
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Mesh %s couldn't be load. Error in '%s' type parser.",
					name.c_str(), typeD->getString().c_str());
				return;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Mesh %s couldn't be load. Unknown type given.", name.c_str());
			return;
		}

		PR_ASSERT(mesh);// After here it shouldn't be null
		env->addMesh(name, mesh);
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
						PR_LOGGER.logf(L_Info, M_Scene, "Temp %f -> Intensity %f",
							grp->at(0)->getFloatConverted(), spec.avg());
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
				return PM::pm_Normalize4D(PM::pm_RotationQuatFromXYZ(x, y, z));
			}
		}

		return PM::pm_IdentityQuat();
	}

	SpectralShaderOutput* SceneLoader::getSpectralOutput(Environment* env, DL::Data* dataD, bool allowScalar) const
	{
		if (!dataD)
			return nullptr;

		if(allowScalar && dataD->isNumber())
		{
			Spectrum spec;
			spec.fill(dataD->getFloatConverted());

			auto* tex = new ConstSpectralShaderOutput(spec);
			env->addShaderOutput(tex);
			
			return tex;
		}
		else if (dataD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(dataD->getString()))
			{
				auto* tex = new ConstSpectralShaderOutput(env->getSpectrum(dataD->getString()));
				env->addShaderOutput(tex);
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

			if((name == "tex" || name == "texture") &&
				dataD->getGroup()->unnamedCount() == 1)
			{
				DL::Data* nameD = dataD->getGroup()->at(0);
				if (nameD->isType() == DL::Data::T_String)
				{
					if(env->hasSpectralShaderOutput(nameD->getString()))
						return env->getSpectralShaderOutput(nameD->getString());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Unknown spectral texture '%s'.",
							nameD->getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}

	ScalarShaderOutput* SceneLoader::getScalarOutput(Environment* env, DL::Data* dataD) const
	{
		if (!dataD)
			return nullptr;

		if (dataD->isNumber())
		{
			auto* tex = new ConstScalarShaderOutput(dataD->getFloatConverted());
			env->addShaderOutput(tex);
			return tex;
		}
		else if (dataD->isType() == DL::Data::T_Group)
		{
			std::string name = dataD->getGroup()->id();

			if((name == "tex" || name == "texture") &&
				dataD->getGroup()->unnamedCount() == 1)
			{
				DL::Data* nameD = dataD->getGroup()->at(0);
				if (nameD->isType() == DL::Data::T_String)
				{
					if(env->hasScalarShaderOutput(nameD->getString()))
						return env->getScalarShaderOutput(nameD->getString());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Unknown scalar texture '%s'.",
							nameD->getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}

	VectorShaderOutput* SceneLoader::getVectorOutput(Environment* env, DL::Data* dataD) const
	{
		if (!dataD)
			return nullptr;
		
		if (dataD->isType() == DL::Data::T_Array)
		{
			bool ok;
			const auto vec = getVector(dataD->getArray(), ok);

			if(ok)
			{
				auto* tex = new ConstVectorShaderOutput(vec);
				env->addShaderOutput(tex);
				return tex;
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid vector entry.");
			}
		}
		else if (dataD->isType() == DL::Data::T_Group)
		{
			std::string name = dataD->getGroup()->id();

			if((name == "tex" || name == "texture") &&
				dataD->getGroup()->unnamedCount() == 1)
			{
				DL::Data* nameD = dataD->getGroup()->at(0);
				if (nameD->isType() == DL::Data::T_String)
				{
					if(env->hasVectorShaderOutput(nameD->getString()))
						return env->getVectorShaderOutput(nameD->getString());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Unknown vector texture '%s'.",
							nameD->getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}
}