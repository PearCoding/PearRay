#pragma once

#include "PR_Config.h"

namespace PR {
namespace VCM {
enum MISMode {
	MM_Balance,
	MM_Power
};

/// Apply the MIS function to the given term
template <MISMode Mode>
inline float mis_term(float a)
{
	if constexpr (Mode == MM_Power)
		return a * a;
	else
		return a;
}
} // namespace VCM
} // namespace PR