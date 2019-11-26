#include "CurveContainer.h"

namespace PR {
BoundingBox CurveContainer::boundingBox(const Curve3& curve, float uMin, float uMax)
{
	Vector3f p0 = curve.evalBlossom({ uMin, uMin, uMin });
	Vector3f p1 = curve.evalBlossom({ uMin, uMin, uMax });
	Vector3f p2 = curve.evalBlossom({ uMin, uMax, uMax });
	Vector3f p3 = curve.evalBlossom({ uMax, uMax, uMax });

	BoundingBox bx(p0, p1);
	bx.combine(p2);
	bx.combine(p3);
	return bx;
}

BoundingBox CurveContainer::constructBoundingBox() const
{
	BoundingBox box;
	for (const Curve3& c : mCurves) {
		for (const auto& p : c.points()) {
			box.combine(p);
		}
	}

	return box;
}

BoundingBox CurveContainer::constructBoundingBox(float uMin, float uMax) const
{
	BoundingBox box;
	for (const Curve3& c : mCurves) {
		Vector3f p0 = c.evalBlossom({ uMin, uMin, uMin });
		Vector3f p1 = c.evalBlossom({ uMin, uMin, uMax });
		Vector3f p2 = c.evalBlossom({ uMin, uMax, uMax });
		Vector3f p3 = c.evalBlossom({ uMax, uMax, uMax });
		box.combine(p0);
		box.combine(p1);
		box.combine(p2);
		box.combine(p3);
	}

	return box;
}

} // namespace PR
