#pragma once

#include "MaterialParser.h"

namespace PRU
{
	class OrenNayarMaterialParser : public IMaterialParser
	{
	public:
		PR::Material* parse(SceneLoader* loader, Environment* env,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
