#pragma once

#include "EntityParser.h"

namespace PR
{
	class PlaneParser : public IEntityParser
	{
	public:
		std::shared_ptr<PR::Entity> parse(Environment* env, const std::string& name,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
