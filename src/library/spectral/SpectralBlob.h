#pragma once

#include "PR_Config.h"

namespace PR {

constexpr size_t SPECTRAL_BLOB_SIZE = 4;

template <typename T>
using SpectralBlobBase = Eigen::Array<T, SPECTRAL_BLOB_SIZE, 1>;

using SpectralBlob = SpectralBlobBase<float>;
//using SpectralBlobV = SpectralBlobBase<vfloat>;

} // namespace PR
