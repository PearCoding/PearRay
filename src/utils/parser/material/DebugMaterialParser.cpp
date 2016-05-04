#include "DebugMaterialParser.h"
#include "material/NormalDebugMaterial.h"
#include "material/UVDebugMaterial.h"

#include "Logger.h"

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
	Material* DebugMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* showD = group->getFromKey("show");

		if (showD && showD->isType() == DL::Data::T_String)
		{
			std::string show = showD->getString();

			if (show == "normal")
			{
				return new NormalDebugMaterial();
			}
			else if (show == "uv")
			{
				return new UVDebugMaterial();
			}
		}

		PR_LOGGER.log(L_Warning, M_Scene, "Couldn't detect debug type from show attribute.");
		return nullptr;
	}
}