#pragma once

#include "Curve.h"
#include "geometry/BoundingBox.h"

namespace PR {
// Container for 3D curves
class PR_LIB CurveContainer {
public:
	CurveContainer() = default;
	inline explicit CurveContainer(const std::vector<Curve3>& curves)
		: mCurves(curves)
	{
	}

	inline const std::vector<Curve3>& curves() const { return mCurves; }
	inline void setCurves(const std::vector<Curve3>& curve) { mCurves = curve; }
	inline const Curve3& curve(size_t i) const { return mCurves[i]; }

	inline size_t nodeCount() const { return mCurves.size(); }

	static inline BoundingBox boundingBox(const Curve3& curve)
	{
		BoundingBox box;
		for (const auto& p : curve.points())
			box.combine(p);
		return box;
	}

	BoundingBox constructBoundingBox() const;

private:
	std::vector<Curve3> mCurves;
};
} // namespace PR
