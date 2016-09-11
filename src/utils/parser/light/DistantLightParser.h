#pragma once

#include "LightParser.h"

namespace PRU
{
	class DistantLightParser : public ILightParser
	{
	public:
		PR::IInfiniteLight* parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const override;
	};
}
