#pragma once

#include "MaterialParser.h"

namespace PRU
{
	class BlinnPhongMaterialParser : public MaterialParser
	{
	public:
		PR::Material* parse(SceneLoader* loader, Environment* env,
			const std::string& obj, DL::DataGroup* group) const override;
	};
}
