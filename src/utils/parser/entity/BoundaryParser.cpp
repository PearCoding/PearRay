#include "BoundaryParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "entity/BoundaryEntity.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Entity> BoundaryParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data materialD = group.getFromKey("material");
		DL::Data sizeD = group.getFromKey("size");

		PM::vec3 size = PM::pm_Set(1, 1, 1);

		if (sizeD.type() == DL::Data::T_Group)
		{
			bool ok;
			size = loader->getVector(sizeD.getGroup(), ok);

			if (!ok)
			{
				size = PM::pm_Set(1, 1, 1);
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid size. Assuming unit cube.", name.c_str());
			}
		}
		else if(sizeD.isNumber())
		{
			size = PM::pm_FillVector3D(sizeD.getNumber());
		}
		else
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has no size. Assuming unit cube.", name.c_str());
		}

		auto bnd = std::make_shared<BoundaryEntity>(env->scene().entities().size()+1, name,
			BoundingBox(PM::pm_GetX(size), PM::pm_GetY(size), PM::pm_GetZ(size)));

		if (materialD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialD.getString()))
				bnd->setMaterial(env->getMaterial(materialD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD.getString().c_str());
		}

		return bnd;
	}
}