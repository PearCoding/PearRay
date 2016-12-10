#pragma once

#include "EntityParser.h"

namespace PRU
{
	class CameraParser : public IEntityParser
	{
	public:
		PR::Entity* parse(SceneLoader* loader, Environment* env, const std::string& name,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
