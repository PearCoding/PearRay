#pragma once

#include "AbstractDatabase.h"

namespace PR {
class IEmission;
class IEntity;
class IInfiniteLight;
class IMaterial;
class INode;

using EmissionDatabase		= NamedDatabase<IEmission>;
using EntityDatabase		= AnonymousDatabase<IEntity>;
using InfiniteLightDatabase = AnonymousDatabase<IInfiniteLight>;
using MaterialDatabase		= NamedDatabase<IMaterial>;
using NodeDatabase			= MixedDatabase<INode>;

/// Database containing all big objects in the world/scene
class PR_LIB_CORE SceneDatabase {
public:
	SceneDatabase();
	virtual ~SceneDatabase();

	std::unique_ptr<EmissionDatabase> Emissions;
	std::unique_ptr<MaterialDatabase> Materials;
	std::unique_ptr<InfiniteLightDatabase> InfiniteLights;
	std::unique_ptr<EntityDatabase> Entities;
	std::unique_ptr<NodeDatabase> Nodes;
};
} // namespace PR