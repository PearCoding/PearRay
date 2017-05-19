#include "PlaneParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "entity/PlaneEntity.h"

#include "DataLisp.h"

namespace PR
{
	std::shared_ptr<PR::Entity> PlaneParser::parse(Environment* env, const std::string& name,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data materialD = group.getFromKey("material");
		DL::Data centeringD = group.getFromKey("centering");

		DL::Data xAxisD = group.getFromKey("xAxis");
		DL::Data yAxisD = group.getFromKey("yAxis");

		Eigen::Vector3f xAxis(1, 0, 0);
		if (xAxisD.type() == DL::Data::T_Group)
		{
			bool ok;
			xAxis = SceneLoader::getVector(xAxisD.getGroup(), ok);

			if (!ok)
			{
				xAxis = Eigen::Vector3f(1, 0, 0);
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid x axis. Assuming unit x vector.", name.c_str());
			}
		}
		else if(xAxisD.isNumber())
		{
			xAxis = Eigen::Vector3f(xAxisD.getNumber(), 0, 0);
		}

		Eigen::Vector3f yAxis(0, 1, 0);
		if (yAxisD.type() == DL::Data::T_Group)
		{
			bool ok;
			yAxis = SceneLoader::getVector(yAxisD.getGroup(), ok);

			if (!ok)
			{
				yAxis = Eigen::Vector3f(0, 1, 0);
				PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has invalid y axis. Assuming unit y vector.", name.c_str());
			}
		}
		else if(yAxisD.isNumber())
		{
			yAxis = Eigen::Vector3f(0, yAxisD.getNumber(), 0);
		}

		Eigen::Vector3f pos(0, 0, 0);
		if(centeringD.type() == DL::Data::T_Bool && centeringD.getBool())
			pos = -xAxis*0.5f - yAxis*0.5f;

		auto entity = std::make_shared<PlaneEntity>(env->scene().entities().size()+1, name,
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