#include "Profiler.h"

namespace PR {
namespace Profiler {
static bool sUseNetwork = false;
void start(int32 networkPort)
{
	sUseNetwork = networkPort >= 0;
    // TODO
}
void stop()
{
    // TODO
}
} // namespace Profiler
} // namespace PR
