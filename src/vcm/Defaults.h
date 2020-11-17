#pragma once

namespace PR {
constexpr float SHADOW_RAY_MIN = 0.0001f;
constexpr float SHADOW_RAY_MAX = PR_INF;
constexpr float BOUNCE_RAY_MIN = SHADOW_RAY_MIN;
constexpr float BOUNCE_RAY_MAX = PR_INF;
constexpr float DISTANCE_EPS   = 0.00001f;
constexpr float GEOMETRY_EPS   = 0.00001f;
constexpr float GATHER_EPS	   = 0.00001f;
} // namespace PR