#pragma once

#include "PR_Config.h"

namespace PR {
namespace Profiler {
void PR_LIB start(int32 networkPort = -1);
void PR_LIB stop();
} // namespace Profiler
} // namespace PR
