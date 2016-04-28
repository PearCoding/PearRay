#include "SphereParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/SphereEntity.h"

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
	Entity* SphereParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* materialD = group->getFromKey("material");
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

		return sphere;
	}
}