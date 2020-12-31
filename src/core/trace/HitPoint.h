#pragma once

#include "Enum.h"

namespace PR {

enum HitPointFlag : uint32 {
	Successful = 0x1
};
PR_MAKE_FLAGS(HitPointFlag, HitPointFlags)

struct PR_LIB_CORE HitPoint {
	float HitDistance	= PR_INF;
	uint32 MaterialID	= 0;
	uint32 EntityID		= 0;
	uint32 FaceID		= 0;
	Vector3f Parameter	= { 0, 0, 0 };
	HitPointFlags Flags = 0;

	inline bool isSuccessful() const { return Flags & HitPointFlag::Successful; }
	inline void makeSuccessful() { Flags |= HitPointFlag::Successful; }
	inline void resetSuccessful() { Flags &= ~(uint32)HitPointFlag::Successful; }
};
} // namespace PR