#pragma once

#include "Enum.h"

namespace PR {
enum class OutputFeedback {
	NaN				= 0x1,
	Infinite		= 0x2,
	Negative		= 0x4,
	MissingMaterial = 0x10,
	MissingEmission = 0x20
};
PR_MAKE_FLAGS(OutputFeedback, OutputFeedbackFlags)
} // namespace PR
