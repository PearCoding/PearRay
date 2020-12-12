#pragma once

namespace PR {
constexpr float SHADOW_RAY_MIN = 0.0001f;
constexpr float SHADOW_RAY_MAX = PR_INF;
constexpr float BOUNCE_RAY_MIN = SHADOW_RAY_MIN;
constexpr float BOUNCE_RAY_MAX = PR_INF;

// Experimental determined constants
constexpr float DISTANCE_EPS = 1e-5f;
constexpr float GEOMETRY_EPS = 1e-5f;
constexpr float GATHER_EPS	 = 1e-5f;
constexpr float PDF_EPS		 = 1e-6f;
constexpr float MIS_EPS		 = 1e-6f;
} // namespace PR