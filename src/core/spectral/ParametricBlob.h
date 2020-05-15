#pragma once

#include "PR_Config.h"

namespace PR {

constexpr size_t PR_PARAMETRIC_BLOB_SIZE = 4;

template <typename T>
using ParametricBlobBase = Eigen::Array<T, PR_PARAMETRIC_BLOB_SIZE, 1>;

using ParametricBlob = ParametricBlobBase<float>;

} // namespace PR
