#include "LightPath.h"

namespace PR {
LightPath::LightPath(size_t expectedSize)
	: mCurrentPos(0)
{
	mTokens.reserve(expectedSize);
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

LightPath LightPath::createCB()
{
	LightPath path(2);
	path.addToken(LightPathToken(ST_CAMERA, SE_NONE));
	path.addToken(LightPathToken(ST_BACKGROUND, SE_NONE));
	return path;
}
} // namespace PR