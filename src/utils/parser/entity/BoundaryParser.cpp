#include "BoundaryParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "entity/BoundaryEntity.h"

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
	Entity* BoundaryParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* materialD = group->getFromKey("material");
		DL::Data* sizeD = group->getFromKey("size");

		PM::vec3 size = PM::pm_Set(1, 1, 1, 1);

		if (sizeD && (sizeD->isType() == DL::Data::T_Array || sizeD->isNumber()))
		{
			if (sizeD->isType() == DL::Data::T_Array)
			{
				DL::DataArray* arr = sizeD->getArray();

				bool ok;
				size = loader->getVector(arr, ok);

				if (!ok)
				{
					size = PM::pm_Set(1, 1, 1, 1);
					PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid size. Assuming unit cube.", name.c_str());
				}
			}
			else
			{
				size = PM::pm_Set(sizeD->getFloatConverted(),
					sizeD->getFloatConverted(),
					sizeD->getFloatConverted(),
					1);
			}
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has no size. Assuming unit cube.", name.c_str());
		}

		BoundaryEntity* bnd = new BoundaryEntity(name,
			BoundingBox(PM::pm_GetX(size), PM::pm_GetY(size), PM::pm_GetZ(size)),
			parent);

		if (materialD && materialD->isType() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialD->getString()))
			{
				bnd->setMaterial(env->getMaterial(materialD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD->getString().c_str());
			}
		}

		return bnd;
	}
}