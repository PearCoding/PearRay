#include "Environment.h"

#include "geometry/Mesh.h"
#include "material/Material.h"

namespace PRU
{
	Environment::Environment(const std::string& name) :
		mScene(name)
	{
	}

	Environment::~Environment()
	{
		mScene.clear();
		for (std::pair<std::string, PR::Material*> p : mMaterials)
		{
			delete p.second;
		}

		for (std::pair<std::string, PR::Mesh*> p : mMeshes)
		{
			delete p.second;
		}
	}
}