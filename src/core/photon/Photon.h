#pragma once

#include "PR_Config.h"

namespace PR {
namespace Photon {
struct alignas(16) Photon {
	float Position[3];
	float Direction[3];
	float Power[3];
};
}
}
