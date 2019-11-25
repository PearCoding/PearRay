#include "CurveContainer.h"

namespace PR {
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

} // namespace PR
