// IWYU pragma: private, include "PR_Config.h"

namespace PR {
constexpr float PR_EPSILON	   = std::numeric_limits<float>::epsilon();
constexpr float PR_INF		   = std::numeric_limits<float>::infinity();
constexpr uint32 PR_INVALID_ID = std::numeric_limits<uint32>::max();

constexpr float PR_PI		= 3.14159265358979323846;
constexpr float PR_INV_PI	= 0.31830988618379067154; // 1/pi
constexpr float PR_INV_2_PI = 0.15915494309189533577; // 1/(2pi)
constexpr float PR_INV_4_PI = 0.07957747154594766788; // 1/(4pi)
constexpr float PR_PI_2		= 1.57079632679489661923; // pi half
constexpr float PR_PI_4		= 0.78539816339744830961; // pi quarter
constexpr float PR_SQRT2	= 1.41421356237309504880;

constexpr float PR_DEG2RAD = PR_PI / 180.0f;
constexpr float PR_RAD2DEG = 180.0f * PR_INV_PI;

constexpr float PR_NM_TO_M_F = 1e-9f;
constexpr double PR_NM_TO_M	 = 1e-9;
} // namespace PR