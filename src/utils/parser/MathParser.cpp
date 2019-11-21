#include "MathParser.h"

#include "DataLisp.h"

namespace PR {
Eigen::Matrix4f MathParser::getMatrix(const DL::DataGroup& grp, bool& ok)
{
	ok = false;
	if (grp.anonymousCount() == 16) {
		ok = true;
		for (int i = 0; i < 16; ++i) {
			if (!grp.at(i).isNumber()) {
				ok = false;
				break;
			}
		}

		if (ok) {
			Eigen::Matrix4f m;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					m(i, j) = grp.at(i * 4 + j).getNumber();
				}
			}

			return m;
		}
	}

	return Eigen::Matrix4f::Identity();
}

Vector3f MathParser::getVector(const DL::DataGroup& arr, bool& ok)
{
	Vector3f res(0, 0, 0);

	if (arr.anonymousCount() == 2) {
		if (arr.at(0).isNumber() && arr.at(1).isNumber()) {
			res = Vector3f(arr.at(0).getNumber(),
						   arr.at(1).getNumber(),
						   0);

			ok = true;
		} else {
			ok = false;
		}
	} else if (arr.anonymousCount() == 3) {
		if (arr.at(0).isNumber() && arr.at(1).isNumber() && arr.at(2).isNumber()) {
			res = Vector3f(arr.at(0).getNumber(),
						   arr.at(1).getNumber(),
						   arr.at(2).getNumber());

			ok = true;
		} else {
			ok = false;
		}
	} else {
		ok = false;
	}

	return res;
}

Eigen::Quaternionf MathParser::getRotation(const DL::Data& data, bool& ok)
{
	if (data.type() == DL::DT_Group) {
		DL::DataGroup grp = data.getGroup();
		if (grp.isArray() && grp.anonymousCount() == 4) {
			if (grp.at(0).isNumber() && grp.at(1).isNumber() && grp.at(2).isNumber() && grp.at(3).isNumber()) {
				ok = true;
				return Eigen::Quaternionf(grp.at(0).getNumber(),
										  grp.at(1).getNumber(),
										  grp.at(2).getNumber(),
										  grp.at(3).getNumber());
			} else {
				ok = false;
			}
		} else if (grp.id() == "euler" && grp.anonymousCount() == 3 && grp.at(0).isNumber() && grp.at(1).isNumber() && grp.at(2).isNumber()) {
			float x = grp.at(0).getNumber() * PR_PI / 180;
			float y = grp.at(1).getNumber() * PR_PI / 180;
			float z = grp.at(2).getNumber() * PR_PI / 180;

			Eigen::AngleAxisf ax(x, Vector3f::UnitX());
			Eigen::AngleAxisf ay(y, Vector3f::UnitY());
			Eigen::AngleAxisf az(z, Vector3f::UnitZ());

			ok = true;
			return az * ay * ax;
		}
	}

	return Eigen::Quaternionf::Identity();
}
} // namespace PR
