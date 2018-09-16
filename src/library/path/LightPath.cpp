#include "LightPath.h"

namespace PR {
LightPath::LightPath(size_t expectedSize)
{
	mTokens.reserve(expectedSize);
}

void LightPath::addToken(const LightPathToken& token)
{
	mTokens.emplace_back(token);
}
} // namespace PR