#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "Data.h"
#include "SourceLogger.h"

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
		// TODO!!!
		return load(path);
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
				for (DL::DataGroup* entry : entries)
				{
					if (entry->id() == "material")
					{
						addMaterial(entry, env);
					}
					else if (entry->id() == "texture")
					{
						//addTexture(entry, env);
					}
					else if (entry->id() == "spectrum")
					{
						addSpectrum(entry, env);
					}
					else if (entry->id() == "mesh")
					{
						addMesh(entry, env);
					}
				}

				// Now entities.
				for (DL::DataGroup* entry : entries)
				{
					if (entry->id() == "entity")
					{
						addEntity(entry, nullptr, env);
					}
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

		std::string name;
		if (nameD && nameD->isType() == DL::Data::T_String)
		{
			name = nameD->getString();
		}
		else
		{
			name = "UNKNOWN";
		}

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
			DL::Data* material = group->getFromKey("material");
			// TODO
		}
	}

	void SceneLoader::addMaterial(DL::DataGroup* group, Environment* env)
	{

	}

	void SceneLoader::addSpectrum(DL::DataGroup* group, Environment* env)
	{

	}

	void SceneLoader::addMesh(DL::DataGroup* group, Environment* env)
	{

	}
}