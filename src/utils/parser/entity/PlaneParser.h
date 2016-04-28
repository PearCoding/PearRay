#pragma once

#include "EntityParser.h"

namespace PRU
{
	class PlaneParser : public EntityParser
	{
	public:
		PR::Entity* parse(SceneLoader* loader, Environment* env, const std::string& name, PR::Entity* parent,
			const std::string& obj, DL::DataGroup* group) const override;
	};
}
