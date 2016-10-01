#include "MeshParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/MeshEntity.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

using namespace PR;
namespace PRU
{
	Entity* MeshParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* materialD = group->getFromKey("material");
		DL::Data* meshD = group->getFromKey("mesh");

		MeshEntity* me = new MeshEntity(name);

		if (meshD && meshD->isType() == DL::Data::T_String)
		{
			if (env->hasMesh(meshD->getString()))
			{
				me->setMesh(env->getMesh(meshD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find mesh %s.", meshD->getString().c_str());
				delete me;
				return nullptr;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Invalid mesh entry found.");
			delete me;
			return nullptr;
		}
		
		if (materialD && materialD->isType() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialD->getString()))
				me->setMaterialOverride(env->getMaterial(materialD->getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
		}

		return me;
	}
}