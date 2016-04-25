#include "DiffuseMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/DiffuseMaterial.h"

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
	Material* DiffuseMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group)
	{

		DL::Data* reflectanceD = group->getFromKey("reflectance");
		DL::Data* emissionD = group->getFromKey("emission");
		DL::Data* roughnessD = group->getFromKey("roughness");
		DL::Data* shadingD = group->getFromKey("shading");
		DL::Data* lightD = group->getFromKey("light");
		DL::Data* selfShadowD = group->getFromKey("selfShadow");
		DL::Data* cameraVisibleD = group->getFromKey("cameraVisible");

		DiffuseMaterial* diff = new DiffuseMaterial;

		if (reflectanceD && reflectanceD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(reflectanceD->getString()))
			{
				diff->setReflectance(env->getSpectrum(reflectanceD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					reflectanceD->getString().c_str());
			}
		}

		if (emissionD && emissionD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(emissionD->getString()))
			{
				diff->setEmission(env->getSpectrum(emissionD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					emissionD->getString().c_str());
			}
		}

		if (roughnessD && roughnessD->isNumber())
		{
			diff->setRoughness(roughnessD->getFloatConverted());
		}

		if (shadingD && shadingD->isType() == DL::Data::T_Bool)
		{
			diff->enableShading(shadingD->getBool());
		}

		if (lightD && lightD->isType() == DL::Data::T_Bool)
		{
			diff->enableLight(lightD->getBool());
		}

		if (selfShadowD && selfShadowD->isType() == DL::Data::T_Bool)
		{
			diff->enableSelfShadow(selfShadowD->getBool());
		}

		if (cameraVisibleD && cameraVisibleD->isType() == DL::Data::T_Bool)
		{
			diff->enableCameraVisibility(cameraVisibleD->getBool());
		}

		return diff;
	}
}