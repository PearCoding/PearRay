#include "SceneDatabase.h"

#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "material/IMaterial.h"
#include "shader/INode.h"

namespace PR {
SceneDatabase::SceneDatabase()
	: Emissions(new EmissionDatabase())
	, Materials(new MaterialDatabase())
	, InfiniteLights(new InfiniteLightDatabase())
	, Entities(new EntityDatabase())
	, Nodes(new NodeDatabase())
{
}

SceneDatabase::~SceneDatabase()
{
}
} // namespace PR