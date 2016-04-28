#include "LightParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "lights/PointLight.h"

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
	Entity* LightParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group) const
	{
		if (obj == "pointLight")
		{
			DL::Data* materialD = group->getFromKey("material");
			PointLight* l = new PointLight(name, parent);

			if (materialD && materialD->isType() == DL::Data::T_String)
			{
				if (env->hasMaterial(materialD->getString()))
				{
					l->setMaterial(env->getMaterial(materialD->getString()));
				}
				else
				{
					PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
				}
			}

			return l;
		}
		else
		{
			return nullptr;
		}
	}
}