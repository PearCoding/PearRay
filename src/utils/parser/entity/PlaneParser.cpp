#include "PlaneParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/PlaneEntity.h"

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
	Entity* PlaneParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data materialD = group->getFromKey("material");
		DL::Data centeringD = group->getFromKey("centering");

		DL::Data xAxisD = group->getFromKey("xAxis");
		DL::Data yAxisD = group->getFromKey("yAxis");

		PM::vec3 xAxis = PM::pm_Set(1, 0, 0, 1);
		if (xAxisD.type() == DL::Data::T_Array)
		{
			DL::DataArray* arr = xAxisD.getArray();

			bool ok;
			xAxis = loader->getVector(arr, ok);

			if (!ok)
			{
				xAxis = PM::pm_Set(1, 0, 0, 1);
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid x axis. Assuming unit x vector.", name.c_str());
			}
		}
		else if(xAxisD.isNumber())
		{
			xAxis = PM::pm_Set(xAxisD.getNumber(), 0, 0, 1);
		}

		PM::vec3 yAxis = PM::pm_Set(0, 1, 0, 1);
		if (yAxisD.type() == DL::Data::T_Array)
		{
			DL::DataArray* arr = yAxisD.getArray();

			bool ok;
			yAxis = loader->getVector(arr, ok);

			if (!ok)
			{
				yAxis = PM::pm_Set(0, 1, 0, 1);
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid y axis. Assuming unit y vector.", name.c_str());
			}
		}
		else if(yAxisD.isNumber())
		{
			yAxis = PM::pm_Set(0, yAxisD.getNumber(), 0, 1);
		}

		PM::vec3 pos = PM::pm_Set(0, 0, 0, 1);
		if(centeringD.type() == DL::Data::T_Bool && centeringD.getBool())
			pos = PM::pm_Add(PM::pm_Scale(xAxis, -0.5f), PM::pm_Scale(yAxis, -0.5f));

		PlaneEntity* entity = new PlaneEntity(env->scene()->entities().size()+1, name,
			Plane(pos, xAxis, yAxis));

		if (materialD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialD.getString()))
				entity->setMaterial(env->getMaterial(materialD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD.getString().c_str());
		}

		return entity;
	}
}