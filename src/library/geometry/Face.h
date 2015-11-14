#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Face
	{
	public:
		Face();
		~Face();

		PM::vec3 V1;
		PM::vec3 V2;
		PM::vec3 V3;

		PM::vec3 N1;
		PM::vec3 N2;
		PM::vec3 N3;

		PM::vec2 UV1;
		PM::vec2 UV2;
		PM::vec2 UV3;
	};
}