#pragma once

#include "PR_Config.h"

namespace DL {
class DataGroup;
class Data;
}

namespace PR {
class Environment;
class MathParser {
public:
	static Vector3f getVector(const DL::DataGroup& arr, bool& ok);
	static Eigen::Matrix4f getMatrix(const DL::DataGroup& arr, bool& ok);
	static Eigen::Quaternionf getRotation(const DL::Data& data, bool& ok);
};
} // namespace PR
