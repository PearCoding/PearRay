#include "GridMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/GridMaterial.h"

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
	Material* GridMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data firstD = group->getFromKey("first");
		DL::Data secondD = group->getFromKey("second");
		DL::Data gridCountD = group->getFromKey("gridCount");
		DL::Data tileUVD = group->getFromKey("tileUV");

		GridMaterial* bnd = new GridMaterial(env->materialCount() + 1);

		if (firstD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(firstD.getString()))
			{
				bnd->setFirstMaterial(env->getMaterial(firstD.getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", firstD.getString().c_str());
			}
		}

		if (secondD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(secondD.getString()))
			{
				bnd->setSecondMaterial(env->getMaterial(secondD.getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", secondD.getString().c_str());
			}
		}

		if (gridCountD.type() == DL::Data::T_Integer)
			bnd->setGridCount(gridCountD.getInt());

		if (tileUVD.type() == DL::Data::T_Bool)
			bnd->setTileUV(tileUVD.getBool());

		return bnd;
	}
}