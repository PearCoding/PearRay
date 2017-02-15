#include "MeshParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/MeshEntity.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	Entity* MeshParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data materialsD = group.getFromKey("materials");
		DL::Data meshD = group.getFromKey("mesh");

		MeshEntity* me = new MeshEntity(env->scene()->entities().size()+1, name);

		if (meshD.type() == DL::Data::T_String)
		{
			if (env->hasMesh(meshD.getString()))
			{
				me->setMesh(env->getMesh(meshD.getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Couldn't find mesh %s.", meshD.getString().c_str());
				delete me;
				return nullptr;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "Invalid mesh entry found.");
			delete me;
			return nullptr;
		}
		
		if (materialsD.type() == DL::Data::T_String)
		{
			me->reserveMaterialSlots(1);
			if (env->hasMaterial(materialsD.getString()))
				me->setMaterial(0, env->getMaterial(materialsD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialsD.getString().c_str());
		}
		else if (materialsD.type() == DL::Data::T_Group)
		{
			const DL::DataGroup& group =  materialsD.getGroup();
			me->reserveMaterialSlots(group.anonymousCount());
			
			uint32 slot = 0;
			for(uint32 i = 0; i < group.anonymousCount(); ++i)
			{
				DL::Data matD = group.at(i);
				if (matD.type() == DL::Data::T_String)
				{
					if (env->hasMaterial(matD.getString()))
						me->setMaterial(slot, env->getMaterial(matD.getString()));
					else
						PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", matD.getString().c_str());
					
					++slot;
				}
			}
		}

		return me;
	}
}