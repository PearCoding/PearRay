#pragma once

#include "LightParser.h"

namespace PR {
class EnvironmentLightParser : public ILightParser {
public:
	std::shared_ptr<PR::IInfiniteLight> parse(
		Environment* env, const DL::DataGroup& group) const override;
};
} // namespace PR
