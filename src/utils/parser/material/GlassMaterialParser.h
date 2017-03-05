#pragma once

#include "MaterialParser.h"

namespace PR
{
	class GlassMaterialParser : public IMaterialParser
	{
	public:
		std::shared_ptr<PR::Material> parse(Environment* env,
			const std::string& obj, const DL::DataGroup& group) const override;
	};
}
