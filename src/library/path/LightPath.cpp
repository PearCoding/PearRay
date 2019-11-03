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

void LightPath::concat(const LightPath& other)
{
	for (size_t i = 0; i < other.currentSize(); ++i)
		addToken(other.token(i));
}

LightPath LightPath::createCDL(uint32 diffuseCount)
{
	LightPath path(2 + diffuseCount);

	path.addToken(LightPathToken(ST_CAMERA, SE_NONE));

	for (uint32 i = 0; i < diffuseCount; ++i)
		path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));

	path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
	return path;
}
} // namespace PR