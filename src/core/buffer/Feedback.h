#pragma once

#include "PR_Config.h"

namespace PR {
enum OutputFeedback {
	OF_NaN			   = 0x1,
	OF_Infinite		   = 0x2,
	OF_Negative		   = 0x4,
	OF_MissingMaterial = 0x10,
	OF_MissingEmission = 0x20
};
} // namespace PR
