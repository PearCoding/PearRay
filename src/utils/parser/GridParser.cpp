#include "GridParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/GridEntity.h"

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
	Entity* GridParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group)
	{
		DL::Data* firstMaterialD = group->getFromKey("firstMaterial");
		DL::Data* secondMaterialD = group->getFromKey("secondMaterial");

		DL::Data* gridCountD = group->getFromKey("gridCount");
		DL::Data* xAxisD = group->getFromKey("xAxis");
		DL::Data* yAxisD = group->getFromKey("yAxis");
		
		PM::vec3 xAxis = PM::pm_Set(1, 0, 0, 1);
		if (xAxisD && (xAxisD->isType() == DL::Data::T_Array || xAxisD->isNumber()))
		{
			if (xAxisD->isType() == DL::Data::T_Array)
			{
				DL::DataArray* arr = xAxisD->getArray();

				bool ok;
				xAxis = loader->getVector(arr, ok);

				if (!ok)
				{
					xAxis = PM::pm_Set(1, 0, 0, 1);
					PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid x axis. Assuming unit x vector.", name.c_str());
				}
			}
			else
			{
				xAxis = PM::pm_Set(xAxisD->getFloatConverted(),
					0,
					0,
					1);
			}
		}
		
		PM::vec3 yAxis = PM::pm_Set(0, 1, 0, 1);
		if (yAxisD && (yAxisD->isType() == DL::Data::T_Array || yAxisD->isNumber()))
		{
			if (yAxisD->isType() == DL::Data::T_Array)
			{
				DL::DataArray* arr = yAxisD->getArray();

				bool ok;
				yAxis = loader->getVector(arr, ok);

				if (!ok)
				{
					yAxis = PM::pm_Set(0, 1, 0, 1);
					PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid y axis. Assuming unit y vector.", name.c_str());
				}
			}
			else
			{
				yAxis = PM::pm_Set(0,
					yAxisD->getFloatConverted(),
					0,
					1);
			}
		}

		GridEntity* entity = new GridEntity(name, Plane(PM::pm_Set(0, 0, 0, 1), xAxis, yAxis), parent);

		if (gridCountD && gridCountD->isType() == DL::Data::T_Integer)
		{
			entity->setGridCount(PM::pm_MaxT(1, gridCountD->getInt()));
		}

		if (firstMaterialD && firstMaterialD->isType() == DL::Data::T_String)
		{
			if (env->hasMaterial(firstMaterialD->getString()))
			{
				entity->setFirstMaterial(env->getMaterial(firstMaterialD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", firstMaterialD->getString().c_str());
			}
		}

		if (secondMaterialD && secondMaterialD->isType() == DL::Data::T_String)
		{
			if (env->hasMaterial(secondMaterialD->getString()))
			{
				entity->setSecondMaterial(env->getMaterial(secondMaterialD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", secondMaterialD->getString().c_str());
			}
		}

		if (entity->firstMaterial() && entity->firstMaterial() == entity->secondMaterial())
		{
			PR_LOGGER.log(L_Warning, M_Scene, "First and second material match each other. Use a plane entity instead.");
		}

		return entity;
	}
}