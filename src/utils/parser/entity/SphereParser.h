#pragma once

#include "EntityParser.h"

namespace PRU
{
	class SphereParser : public EntityParser
	{
	public:
		PR::Entity* parse(SceneLoader* loader, Environment* env, const std::string& name, PR::Entity* parent,
			const std::string& obj, DL::DataGroup* group) const override;
	};
}
