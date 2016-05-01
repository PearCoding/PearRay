#include "MirrorMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/MirrorMaterial.h"

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
	Material* MirrorMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* cameraVisibleD = group->getFromKey("cameraVisible");

		MirrorMaterial* diff = new MirrorMaterial;

		if (cameraVisibleD && cameraVisibleD->isType() == DL::Data::T_Bool)
		{
			diff->enableCameraVisibility(cameraVisibleD->getBool());
		}

		return diff;
	}
}