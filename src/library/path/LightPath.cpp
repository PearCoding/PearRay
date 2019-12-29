#include "LightPath.h"

namespace PR {
LightPath::LightPath(size_t expectedSize)
	: mCurrentPos(0)
{
	mTokens.reserve(expectedSize);
}

LightPath LightPath::createCDL(size_t diffuseCount)
{
	LightPath path(2 + diffuseCount);

	path.addToken(LightPathToken::Camera());

	for (size_t i = 0; i < diffuseCount; ++i)
		path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));

	path.addToken(LightPathToken::Emissive());
	return path;
}

LightPath LightPath::createCB()
{
	LightPath path(2);
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken::Background());
	return path;
}
} // namespace PR