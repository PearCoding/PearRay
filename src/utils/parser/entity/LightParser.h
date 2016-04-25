#pragma once

#include "EntityParser.h"

namespace PRU
{
	class LightParser : public EntityParser
	{
	public:
		PR::Entity* parse(SceneLoader* loader, Environment* env, const std::string& name, PR::Entity* parent,
			const std::string& obj, DL::DataGroup* group) override;
	};
}
