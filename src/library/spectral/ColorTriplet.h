#pragma once

#include "PR_Config.h"

namespace PR {

template <typename T>
using ColorTripletBase = Eigen::Array<T, 3, 1>;

typedef ColorTripletBase<float> ColorTriplet;
typedef ColorTripletBase<vfloat> ColorTripletV;

} // namespace PR
