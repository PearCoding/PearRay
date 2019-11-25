#pragma once

#include "curve/Curve.h"

namespace DL {
class DataGroup;
}

namespace PR {

class CurveContainer;
class CurveParser {
public:
	static Curve1 parse1D(const DL::DataGroup& group);
	static Curve2 parse2D(const DL::DataGroup& group);
	static Curve3 parse3D(const DL::DataGroup& group);

	static std::shared_ptr<CurveContainer> parse(const DL::DataGroup& group);
};
} // namespace PR
