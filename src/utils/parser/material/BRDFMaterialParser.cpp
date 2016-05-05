#include "BRDFMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/BRDFMaterial.h"

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
	Material* BRDFMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");
		DL::Data* specD = group->getFromKey("specularity");
		DL::Data* roughnessD = group->getFromKey("roughness");
		DL::Data* reflectivityD = group->getFromKey("reflectivity");
		DL::Data* fresnelD = group->getFromKey("fresnel");

		BRDFMaterial* diff = new BRDFMaterial;

		if (albedoD && albedoD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(albedoD->getString()))
			{
				diff->setAlbedo(env->getSpectrum(albedoD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					albedoD->getString().c_str());
			}
		}

		if (specD && specD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(specD->getString()))
			{
				diff->setSpecularity(env->getSpectrum(specD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					specD->getString().c_str());
			}
		}

		if (roughnessD && roughnessD->isNumber())
		{
			diff->setRoughness(roughnessD->getFloatConverted());
		}

		if (reflectivityD && reflectivityD->isNumber())
		{
			diff->setReflectivity(reflectivityD->getFloatConverted());
		}

		if (fresnelD && fresnelD->isNumber())
		{
			diff->setFresnel(fresnelD->getFloatConverted());
		}

		return diff;
	}
}