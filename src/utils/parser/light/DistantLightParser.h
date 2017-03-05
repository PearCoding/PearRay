#pragma once

#include "LightParser.h"

namespace PR
{
	class DistantLightParser : public ILightParser
	{
	public:
		std::shared_ptr<PR::IInfiniteLight> parse(Environment* env, const DL::DataGroup& group) const override;
	};
}
