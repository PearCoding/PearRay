#pragma once

#include "LightParser.h"

namespace PRU
{
	class EnvironmentLightParser : public ILightParser
	{
	public:
		std::shared_ptr<PR::IInfiniteLight> parse(SceneLoader* loader, Environment* env, const DL::DataGroup& group) const override;
	};
}
