#pragma once

#include "PR_Config.h"

namespace PR {

enum HitPointFlags {
	HPF_SUCCESSFUL = 0x1
};

struct PR_LIB_CORE HitPoint {
	float HitDistance  = PR_INF;
	uint32 MaterialID  = 0;
	uint32 EntityID	   = 0;
	uint32 FaceID	   = 0;
	Vector3f Parameter = { 0, 0, 0 };
	uint32 Flags	   = 0;

	inline bool isSuccessful() const { return Flags & HPF_SUCCESSFUL; }
	inline void makeSuccessful() { Flags |= HPF_SUCCESSFUL; }
	inline void resetSuccessful() { Flags &= ~HPF_SUCCESSFUL; }
};
} // namespace PR