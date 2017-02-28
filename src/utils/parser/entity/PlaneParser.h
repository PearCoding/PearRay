#pragma once

#include "EntityParser.h"

namespace PRU
{
	class PlaneParser : public IEntityParser
	{
	public:
		std::shared_ptr<PR::Entity> parse(SceneLoader* loader, Environment* env, const std::string& name,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
