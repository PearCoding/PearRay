#pragma once

#include "Config.h"

namespace PR
{
	class PR_LIB Vertex
	{
	public:
		float X;
		float Y;
		float Z;

		Vertex(float x = 0, float y = 0, float z = 0);
	};
}