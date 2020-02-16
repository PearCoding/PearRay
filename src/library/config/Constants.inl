// IWYU pragma: private, include "PR_Config.h"

namespace PR {
constexpr float PR_EPSILON = std::numeric_limits<float>::epsilon();

#ifdef M_PI
constexpr float PR_PI = static_cast<float>(M_PI);
#else
constexpr float PR_PI	= static_cast<float>(3.14159265358979323846);
#endif

#ifdef M_1_PI
constexpr float PR_1_PI = M_1_PI;
#else
constexpr float PR_1_PI = 1 / PR_PI;
#endif

constexpr float PR_DEG2RAD = PR_PI / 180.0f;
constexpr float PR_RAD2DEG = 180.0f * PR_1_PI;

constexpr float PR_NM_TO_M_F = 1e-9f;
constexpr double PR_NM_TO_M	 = 1e-9;
} // namespace PR