#pragma once

#include "MaterialParser.h"

namespace PRU
{
	class WardMaterialParser : public MaterialParser
	{
	public:
		PR::Material* parse(SceneLoader* loader, Environment* env,
			const std::string& obj, DL::DataGroup* group) const override;
	};
}