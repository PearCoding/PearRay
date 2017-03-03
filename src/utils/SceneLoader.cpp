#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/Entity.h"
#include "entity/RenderEntity.h"

#include "parser/entity/BoundaryParser.h"
#include "parser/entity/CameraParser.h"
#include "parser/entity/CoordinateAxisParser.h"
#include "parser/entity/MeshParser.h"
#include "parser/entity/PlaneParser.h"
#include "parser/entity/SphereParser.h"

#include "parser/material/BlinnPhongMaterialParser.h"
#include "parser/material/CookTorranceMaterialParser.h"
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

#include "loader/WavefrontLoader.h"

#include "spectral/RGBConverter.h"

#include "material/Material.h"

#include "DataLisp.h"

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
		dataLisp.build(container);

		//PR_LOGGER.log(L_Info, M_Scene, dataLisp.dump());

		auto entries = container.getTopGroups();

		if (entries.empty())
		{
			PR_LOGGER.log(L_Error, M_Scene, "DataLisp file does not contain valid entries");
			return nullptr;
		}
		else
		{
			DL::DataGroup top = entries.front();

			if (top.id() != "scene")
			{
				PR_LOGGER.log(L_Error, M_Scene, "DataLisp file does not contain valid top entry");
				return nullptr;
			}
			else
			{
				Environment* env;
				DL::Data nameD = top.getFromKey("name");
				DL::Data renderWidthD = top.getFromKey("renderWidth");
				DL::Data renderHeightD = top.getFromKey("renderHeight");
				DL::Data cropD = top.getFromKey("crop");

				if (nameD.type() != DL::Data::T_String)
					env = new Environment("UNKNOWN");
				else
					env = new Environment(nameD.getString());

				if (renderWidthD.type() == DL::Data::T_Integer)
				{
					env->setRenderWidth(renderWidthD.getInt());
				}

				if (renderHeightD.type() == DL::Data::T_Integer)
				{
					env->setRenderHeight(renderHeightD.getInt());
				}

				if (cropD.type() == DL::Data::T_Group)
				{
					DL::DataGroup arr = cropD.getGroup();
					if (arr.anonymousCount() == 4)
					{
						if (arr.isAllNumber())
						{
							env->setCrop(arr.at(0).getNumber(), arr.at(1).getNumber(),
								arr.at(2).getNumber(), arr.at(3).getNumber());
						}
					}
				}
				
				// Output information
				env->outputSpecification().parse(this, env, top);

				// First independent information
				for (size_t i = 0; i < top.anonymousCount(); ++i)
				{
					DL::Data dataD = top.at(i);

					if (dataD.type() == DL::Data::T_Group)
					{
						DL::DataGroup entry = dataD.getGroup();

						if (entry.id() == "spectrum")
						{
							addSpectrum(entry, env);
						}
						else if (entry.id() == "texture")
						{
							addTexture(entry, env);
						}
						else if (entry.id() == "graph")
						{
							addSubGraph(entry, env);
						}
						else if (entry.id() == "mesh")
						{
							addMesh(entry, env);
						}
					}
				}

				// Now semi-dependent information
				for (size_t i = 0; i < top.anonymousCount(); ++i)
				{
					DL::Data dataD = top.at(i);

					if (dataD.type() == DL::Data::T_Group)
					{
						DL::DataGroup entry = dataD.getGroup();

						if (entry.id() == "material")
							addMaterial(entry, env);
					}
				}

				// Now entities and lights
				for (size_t i = 0; i < top.anonymousCount(); ++i)
				{
					DL::Data dataD = top.at(i);

					if (dataD.type() == DL::Data::T_Group)
					{
						DL::DataGroup entry = dataD.getGroup();

						if (entry.id() == "entity")
						{
							addEntity(entry, nullptr, env);
						}
						else if(entry.id() == "light")
						{
							addLight(entry, env);
						}
					}
				}
				
				DL::Data cameraD = top.getFromKey("camera");
				if (cameraD.type() == DL::Data::T_String)
				{
					Camera* cam = (Camera*)env->scene().getEntity(cameraD.getString(), "standard_camera").get();
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
		{ "axis", CoordinateAxisParser() },
		{ "mesh", MeshParser() },
		{ "plane", PlaneParser() },
		{ "sphere", SphereParser() },

		{ nullptr, BoundaryParser() },//Just for the end
	};

	void SceneLoader::addEntity(const DL::DataGroup& group, const std::shared_ptr<PR::Entity>& parent, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");
		DL::Data typeD = group.getFromKey("type");
		DL::Data posD = group.getFromKey("position");
		DL::Data rotD = group.getFromKey("rotation");
		DL::Data scaleD = group.getFromKey("scale");

		DL::Data localAreaD = group.getFromKey("local_area");

		std::string name;
		if (nameD.type() == DL::Data::T_String)
			name = nameD.getString();
		else
			name = "UNKNOWN";

		std::shared_ptr<Entity> entity;
		if (typeD.type() != DL::Data::T_String)
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. No valid type given.", name.c_str());
			return;
		}
		else if (typeD.getString() == "null" || typeD.getString() == "empty")
		{
			entity = std::make_shared<Entity>(env->scene().entities().size()+1, name);
		}
		else
		{
			const IEntityParser* parser = nullptr;
			for (int i = 0; EntityParserEntries[i].Name; ++i)
			{
				if (typeD.getString() == EntityParserEntries[i].Name)
				{
					parser = &EntityParserEntries[i].Parser;
					break;
				}
			}

			if (parser)
			{
				entity = parser->parse(this, env, name, typeD.getString(), group);

				if (!entity)
				{
					PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. Error in '%s' type parser.",
						name.c_str(), typeD.getString().c_str());
					return;
				}
			}
			else
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Entity %s couldn't be load. Unknown type given.", name.c_str());
				return;
			}
		}

		PR_ASSERT(entity, "After here it shouldn't be null");

		// Set position		
		if (posD.type() == DL::Data::T_Group)
		{
			bool ok;
			PM::vec3 p = getVector(posD.getGroup(), ok);

			if (!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set position for entity %s.", name.c_str());
			else
				entity->setPosition(p);
		}

		// Set rotation
		{
			bool ok;
			PM::quat rot = getRotation(rotD, ok);

			if (!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set rotation for entity %s.", name.c_str());
			else
				entity->setRotation(rot);
		}

		// Set scale
		if (scaleD.isNumber())
		{
			float s = scaleD.getNumber();
			entity->setScale(PM::pm_Set(s, s, s));
		}
		else if(scaleD.type() == DL::Data::T_Group)
		{
			bool ok;
			PM::vec3 s = getVector(scaleD.getGroup(), ok);

			if(!ok)
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set scale for entity %s.", name.c_str());
			else
				entity->setScale(s);
		}

		if(parent)
		{
			PM::mat4 m = PM::pm_Product(parent->matrix(), entity->matrix());

			PM::vec3 p, s;
			PM::quat r;
			PM::pm_Decompose(m, p, s, r);

			entity->setPosition(p);
			entity->setRotation(r);
			entity->setScale(s);
		}

		if (localAreaD.type() == DL::Data::T_Bool)
		{
			if(localAreaD.getBool())
				entity->setFlags(entity->flags() | EF_LocalArea);
			else
				entity->setFlags(entity->flags() & ~(uint8)EF_LocalArea);
		}

		// Add to scene
		env->scene().addEntity(entity);

		for (size_t i = 0; i < group.anonymousCount(); ++i)
		{
			if (group.at(i).type() == DL::Data::T_Group)
			{
				DL::DataGroup child = group.at(i).getGroup();

				if (child.id() == "entity")
					addEntity(child, entity, env);
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
	void SceneLoader::addLight(const DL::DataGroup& group, Environment* env)
	{
		DL::Data typeD = group.getFromKey("type");

		std::string type;

		if (typeD.type() == DL::Data::T_String)
		{
			type = typeD.getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No light type set");
			return;
		}

		std::shared_ptr<IInfiniteLight> light;

		const ILightParser* parser = nullptr;
		for (int i = 0; LightParserEntries[i].Name; ++i)
		{
			if (typeD.getString() == LightParserEntries[i].Name)
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
					typeD.getString().c_str());
				return;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Light couldn't be load. Unknown type given.");
			return;
		}

		PR_ASSERT(light, "After here it shouldn't be null");
		env->scene().addInfiniteLight(light);
	}

	struct
	{
		const char* Name;
		const IMaterialParser& Parser;
	} MaterialParserEntries[] =
	{
		{ "standard", CookTorranceMaterialParser() },
		{ "light", DiffuseMaterialParser() },

		{ "diffuse", DiffuseMaterialParser() },
		{ "orennayar", OrenNayarMaterialParser() },
		{ "blinnphong", BlinnPhongMaterialParser() },
		{ "ward", WardMaterialParser() },
		{ "cook_torrance", CookTorranceMaterialParser() },

		{ "grid", GridMaterialParser() },
		{ "glass", GlassMaterialParser() },
		{ "mirror", MirrorMaterialParser() },

		{ nullptr, DiffuseMaterialParser() },//Just for the end
	};
	void SceneLoader::addMaterial(const DL::DataGroup& group, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");
		DL::Data typeD = group.getFromKey("type");

		DL::Data shadowD = group.getFromKey("shadow");
		DL::Data selfShadowD = group.getFromKey("self_shadow");
		DL::Data cameraVisibleD = group.getFromKey("camera_visible");
		DL::Data shadeableD = group.getFromKey("shadeable");
		DL::Data emissionD = group.getFromKey("emission");

		std::string name;
		std::string type;

		if (nameD.type() == DL::Data::T_String)
		{
			name = nameD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No material name set");
			return;
		}

		if (typeD.type() == DL::Data::T_String)
		{
			type = typeD.getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No material type set");
			return;
		}

		std::shared_ptr<Material> mat;
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
			mat = parser->parse(this, env, typeD.getString(), group);

			if (!mat)
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Material %s couldn't be load. Error in '%s' type parser.",
					name.c_str(), typeD.getString().c_str());
				return;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Material %s couldn't be load. Unknown type given.", name.c_str());
			return;
		}

		PR_ASSERT(mat, "After here it shouldn't be null");

		mat->setEmission(getSpectralOutput(env, emissionD));

		if (shadeableD.type() == DL::Data::T_Bool)
			mat->enableShading(shadeableD.getBool());

		if (shadowD.type() == DL::Data::T_Bool)
			mat->enableShadow(shadowD.getBool());

		if (mat->isLight())
			mat->enableShadow(true);// Force for lights

		if (selfShadowD.type() == DL::Data::T_Bool)
			mat->enableSelfShadow(selfShadowD.getBool());

		if (cameraVisibleD.type() == DL::Data::T_Bool)
			mat->enableCameraVisibility(cameraVisibleD.getBool());

		env->addMaterial(name, mat);
	}

	void SceneLoader::addTexture(const DL::DataGroup& group, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");

		std::string name;

		if (nameD.type() == DL::Data::T_String)
		{
			name = nameD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No texture name set");
			return;
		}

		TextureParser parser;
		parser.parse(this, env, name, group);// Will be added to env here
	}

	void SceneLoader::addMesh(const DL::DataGroup& group, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");
		DL::Data typeD = group.getFromKey("type");

		std::string name;
		std::string type;

		if (nameD.type() == DL::Data::T_String)
		{
			name = nameD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No mesh name set");
			return;
		}

		if (typeD.type() == DL::Data::T_String)
		{
			type = typeD.getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No mesh type set");
			return;
		}

		TriMeshInlineParser parser;
		auto mesh = parser.parse(this, env, group);

		if (!mesh)
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Mesh %s couldn't be load. Error in '%s' type parser.",
				name.c_str(), typeD.getString().c_str());
			return;
		}

		PR_ASSERT(mesh, "After here it shouldn't be null");
		env->addMesh(name, mesh);
	}

	void SceneLoader::addSpectrum(const DL::DataGroup& group, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");
		DL::Data dataD = group.getFromKey("data");

		std::string name;
		if (nameD.type() == DL::Data::T_String)
		{
			name = nameD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get name for spectral entry.");
			return;
		}

		Spectrum spec;
		if (dataD.type() == DL::Data::T_Group)
		{
			DL::DataGroup grp = dataD.getGroup();
			if(grp.isArray())
			{
				for (size_t i = 0; i < grp.anonymousCount() && i < Spectrum::SAMPLING_COUNT; ++i)
				{
					if (grp.at(i).isNumber())
						spec.setValue((uint32)i, grp.at(i).getNumber());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't set spectrum entry at index %i.", i);
				}
			}
			else if (grp.id() == "field")
			{
				DL::Data defaultD = grp.getFromKey("default");

				if (defaultD.isNumber())
				{
					for (uint32 i = 0; i < PR::Spectrum::SAMPLING_COUNT; ++i)
					{
						spec.setValue(i, defaultD.getNumber());
					}
				}

				for (uint32 i = 0;
					 i < grp.anonymousCount() && i < PR::Spectrum::SAMPLING_COUNT;
					 ++i)
				{
					DL::Data fieldD = grp.at(i);

					if (fieldD.isNumber())
						spec.setValue(i, fieldD.getNumber());
				}
			}
			else if (grp.id() == "rgb")
			{
				if (grp.anonymousCount() == 3 &&
					grp.at(0).isNumber() &&
					grp.at(1).isNumber() &&
					grp.at(2).isNumber())
				{
					spec = RGBConverter::toSpec(grp.at(0).getNumber(),
						grp.at(1).getNumber(),
						grp.at(2).getNumber());
				}
			}
			else if (grp.id() == "xyz")
			{
				// TODO
			}
			else if (grp.id() == "temperature" || grp.id() == "blackbody")// Luminance
			{
				if (grp.anonymousCount() >= 1 &&
					grp.at(0).isNumber())
				{
					spec = Spectrum::fromBlackbody(PM::pm_Max(0.0f, grp.at(0).getNumber()));
					spec.weightPhotometric();

					/*PR_LOGGER.logf(L_Info, M_Scene, "Temp %f -> Luminance %f",
						grp->at(0)->getNumber(), spec.avg());*/
				}

				if (grp.anonymousCount() >= 2 &&
					grp.at(1).isNumber())
				{
					spec *= grp.at(1).getNumber();
				}
			}
			else if (grp.id() == "temperature_raw" || grp.id() == "blackbody_raw")// Radiance
			{
				if (grp.anonymousCount() >= 1 &&
					grp.at(0).isNumber())
				{
					spec = Spectrum::fromBlackbody(PM::pm_Max(0.0f, grp.at(0).getNumber()));

					/*PR_LOGGER.logf(L_Info, M_Scene, "Temp %f -> Radiance %f",
						grp->at(0)->getNumber(), spec.avg());*/
				}

				if (grp.anonymousCount() >= 2 &&
					grp.at(1).isNumber())
				{
					spec *= grp.at(1).getNumber();
				}
			}
			else if (grp.id() == "temperature_norm" || grp.id() == "blackbody_norm")// Luminance Norm
			{
				if (grp.anonymousCount() >= 1 &&
					grp.at(0).isNumber())
				{
					spec = Spectrum::fromBlackbody(PM::pm_Max(0.0f, grp.at(0).getNumber()));
					spec.weightPhotometric();
					spec.normalize();
				}

				if (grp.anonymousCount() >= 2 &&
					grp.at(1).isNumber())
				{
					spec *= grp.at(1).getNumber();
				}
			}
			else if (grp.id() == "temperature_raw_norm" || grp.id() == "blackbody_raw_norm")// Radiance Norm
			{
				if (grp.anonymousCount() >= 1 &&
					grp.at(0).isNumber())
				{
					spec = Spectrum::fromBlackbody(PM::pm_Max(0.0f, grp.at(0).getNumber()));
					spec.normalize();
				}

				if (grp.anonymousCount() >= 2 &&
					grp.at(1).isNumber())
				{
					spec *= grp.at(1).getNumber();
				}
			}
		}

		env->addSpectrum(name, spec);
	}

	void SceneLoader::addSubGraph(const DL::DataGroup& group, Environment* env)
	{
		DL::Data nameD = group.getFromKey("name");
		DL::Data overridesD = group.getFromKey("overrides");
		DL::Data loaderD = group.getFromKey("loader");
		DL::Data fileD = group.getFromKey("file");

		std::string name;
		if (nameD.type() == DL::Data::T_String)
			name = nameD.getString();

		std::map<std::string, std::string> overrides;
		if (overridesD.type() == DL::Data::T_String)
			overrides[""] = overridesD.getString();
		
		std::string file;
		if (fileD.type() == DL::Data::T_String)
		{
			file = fileD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't get file for subgraph entry.");
			return;
		}

		std::string loader;
		if (loaderD.type() == DL::Data::T_String)
		{
			loader = loaderD.getString();
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "No valid loader set. Assuming 'obj'.");
			loader = "obj";
		}

		if (loader == "obj")
		{
			DL::Data flipNormalD = group.getFromKey("flipNormal");
			
			WavefrontLoader loader(overrides);

			if (flipNormalD.type() == DL::Data::T_Bool)
				loader.flipNormal(flipNormalD.getBool());

			loader.load(file, env);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Unknown '%s' loader.", loader.c_str());
		}
	}

	PM::vec3 SceneLoader::getVector(const DL::DataGroup& arr, bool& ok) const
	{
		PM::vec3 res = PM::pm_Set(0, 0, 0);

		if (arr.anonymousCount() == 2)
		{
			if (arr.at(0).isNumber() &&
				arr.at(1).isNumber())
			{
				res = PM::pm_Set(arr.at(0).getNumber(),
					arr.at(1).getNumber(),
					0);

				ok = true;
			}
			else
			{
				ok = false;
			}
		}
		else if (arr.anonymousCount() == 3)
		{
			if (arr.at(0).isNumber() &&
				arr.at(1).isNumber() &&
				arr.at(2).isNumber())
			{
				res = PM::pm_Set(arr.at(0).getNumber(),
					arr.at(1).getNumber(),
					arr.at(2).getNumber());

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
	
	PM::quat SceneLoader::getRotation(const DL::Data& data, bool& ok) const
	{
		if (data.type() == DL::Data::T_Group)
		{
			DL::DataGroup grp = data.getGroup();
			if(grp.isArray() && grp.anonymousCount() == 4)
			{
				if (grp.at(0).isNumber() &&
					grp.at(1).isNumber() &&
					grp.at(2).isNumber() &&
					grp.at(3).isNumber())
				{
					ok = true;
					return PM::pm_Normalize(PM::pm_Set(grp.at(0).getNumber(),
						grp.at(1).getNumber(),
						grp.at(2).getNumber(),
						grp.at(3).getNumber()));
				}
				else
				{
					ok = false;
				}
			}
			else if (grp.id() == "euler" && grp.anonymousCount() == 3 &&
				grp.at(0).isNumber() && grp.at(1).isNumber() && grp.at(2).isNumber())
			{
				float x = PM::pm_DegToRad(grp.at(0).getNumber());
				float y = PM::pm_DegToRad(grp.at(1).getNumber());
				float z = PM::pm_DegToRad(grp.at(2).getNumber());

				ok = true;
				return PM::pm_Normalize(PM::pm_RotationQuatFromXYZ(x, y, z));
			}
		}

		return PM::pm_IdentityQuat();
	}

	std::shared_ptr<SpectralShaderOutput> SceneLoader::getSpectralOutput(Environment* env, const DL::Data& dataD, bool allowScalar) const
	{
		if(allowScalar && dataD.isNumber())
		{
			Spectrum spec;
			spec.fill(dataD.getNumber());

			return std::make_shared<ConstSpectralShaderOutput>(spec);
		}
		else if (dataD.type() == DL::Data::T_String)
		{
			if (env->hasSpectrum(dataD.getString()))
				return std::make_shared<ConstSpectralShaderOutput>(env->getSpectrum(dataD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					dataD.getString().c_str());
		}
		else if (dataD.type() == DL::Data::T_Group)
		{
			std::string name = dataD.getGroup().id();

			if((name == "tex" || name == "texture") &&
				dataD.getGroup().anonymousCount() == 1)
			{
				DL::Data nameD = dataD.getGroup().at(0);
				if (nameD.type() == DL::Data::T_String)
				{
					if(env->hasSpectralShaderOutput(nameD.getString()))
						return env->getSpectralShaderOutput(nameD.getString());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Unknown spectral texture '%s'.",
							nameD.getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}
		else if(dataD.isValid())
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}

	std::shared_ptr<ScalarShaderOutput> SceneLoader::getScalarOutput(Environment* env, const DL::Data& dataD) const
	{
		if (dataD.isNumber())
		{
			return std::make_shared<ConstScalarShaderOutput>(dataD.getNumber());
		}
		else if (dataD.type() == DL::Data::T_Group)
		{
			std::string name = dataD.getGroup().id();

			if((name == "tex" || name == "texture") &&
				dataD.getGroup().anonymousCount() == 1)
			{
				DL::Data nameD = dataD.getGroup().at(0);
				if (nameD.type() == DL::Data::T_String)
				{
					if(env->hasScalarShaderOutput(nameD.getString()))
						return env->getScalarShaderOutput(nameD.getString());
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Unknown scalar texture '%s'.",
							nameD.getString().c_str());
				}
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
			}
		}
		else if(dataD.isValid())
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}

	std::shared_ptr<VectorShaderOutput> SceneLoader::getVectorOutput(Environment* env, const DL::Data& dataD) const
	{		
		if (dataD.type() == DL::Data::T_Group)
		{
			if(dataD.getGroup().isArray())
			{
				bool ok;
				const auto vec = getVector(dataD.getGroup(), ok);

				if(ok)
				{
					return std::make_shared<ConstVectorShaderOutput>(vec);
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Invalid vector entry.");
				}
			}
			else
			{
				std::string name = dataD.getGroup().id();

				if((name == "tex" || name == "texture") &&
					dataD.getGroup().anonymousCount() == 1)
				{
					DL::Data nameD = dataD.getGroup().at(0);
					if (nameD.type() == DL::Data::T_String)
					{
						if(env->hasVectorShaderOutput(nameD.getString()))
							return env->getVectorShaderOutput(nameD.getString());
						else
							PR_LOGGER.logf(L_Warning, M_Scene, "Unknown vector texture '%s'.",
								nameD.getString().c_str());
					}
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Unknown data entry.");
				}
			}
		}
		else if(dataD.isValid())
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Unknown texture entry.");
		}

		return nullptr;
	}
}