#pragma once

#include "MaterialParser.h"

namespace PRU
{
	class BlinnPhongMaterialParser : public IMaterialParser
	{
	public:
		std::shared_ptr<PR::Material> parse(SceneLoader* loader, Environment* env,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
