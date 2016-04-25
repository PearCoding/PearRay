#include "DebugMaterialParser.h"
#include "material/DebugMaterial.h"

using namespace PR;
namespace PRU
{
	Material* DebugMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group)
	{
		return new DebugMaterial();
	}
}