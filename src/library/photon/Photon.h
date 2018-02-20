#pragma once

#include "PR_Config.h"

namespace PR {
namespace Photon {
struct alignas(16) Photon {
	float Position[3];
	uint8 Phi, Theta; // Support of 65536 possible directions is enough
					  //uint8 KDFlags;// Flags for the KD-Tree
	float Power[3];
};
}
}
