#pragma once

#include "EntityParser.h"

namespace PRU
{
	class BoundaryParser : public IEntityParser
	{
	public:
		PR::Entity* parse(SceneLoader* loader, Environment* env, const std::string& name,
			const std::string& obj, DL::DataGroup* group) const override;
	};
}
