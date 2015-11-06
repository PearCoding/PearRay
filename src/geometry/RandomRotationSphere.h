#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class RandomRotationSphere
	{
		PR_CLASS_NON_COPYABLE(RandomRotationSphere);
	public:
		static PM::vec3 create(const PM::vec3& normal, float sph, float eph, float srh, float erh);
	};
}