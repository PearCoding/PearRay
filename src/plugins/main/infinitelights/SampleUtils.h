#pragma once

#include "math/Sampling.h"
#include "math/Tangent.h"

namespace PR {
/// Based on a given outgoing direction sample a position which samples the whole scene visible by the bounding sphere
inline Vector3f sampleVisibleHemispherePos(const Vector2f& rnd, const Vector3f& outgoing, float radius, float& pdfA)
{
	const float dist		  = 2 * radius;
	constexpr float CosAtan05 = 0.894427190999915f; // cos(atan(0.5)) = 2/sqrt(5)
	const Vector3f local	  = Sampling::uniform_cone(rnd(0), rnd(1), CosAtan05);
	pdfA					  = Sampling::uniform_cone_pdf(CosAtan05) / (dist * dist);
	return dist * Tangent::align(outgoing, local);
}
} // namespace PR