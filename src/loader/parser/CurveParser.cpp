#include "CurveParser.h"
#include "Logger.h"
#include "MathParser.h"

#include "DataLisp.h"

namespace PR {

Curve1 CurveParser::parse1D(const DL::DataGroup& group)
{
	Curve1 curve;
	if (!group.isAllAnonymousNumber() || group.anonymousCount() < 2)
		return curve;

	curve.points().reserve(group.anonymousCount());
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		float f = group.at(i).getNumber();
		curve.points().push_back(f);
	}

	return curve;
}

Curve2 CurveParser::parse2D(const DL::DataGroup& group)
{
	Curve2 curve;
	if (!group.isAllAnonymousOfType(DL::DT_Group) || group.anonymousCount() < 2)
		return curve;

	curve.points().reserve(group.anonymousCount());
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		bool ok;
		Vector3f vf = MathParser::getVector(group.at(i).getGroup(), ok);
		if (!ok)
			return Curve2();
		curve.points().push_back(Vector2f(vf(0), vf(1)));
	}

	return curve;
}

Curve3 CurveParser::parse3D(const DL::DataGroup& group)
{
	Curve3 curve;
	if (!group.isAllAnonymousOfType(DL::DT_Group) || group.anonymousCount() < 2)
		return curve;

	curve.points().reserve(group.anonymousCount());
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		bool ok;
		Vector3f vf = MathParser::getVector(group.at(i).getGroup(), ok);
		if (!ok)
			return Curve3();
		curve.points().push_back(vf);
	}

	return curve;
}
} // namespace PR
