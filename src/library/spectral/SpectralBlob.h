#pragma once

#include "PR_Config.h"

namespace PR {

constexpr size_t PR_SPECTRAL_BLOB_SIZE = 4;

template <typename T>
using SpectralBlobBase = Eigen::Array<T, PR_SPECTRAL_BLOB_SIZE, 1>;

using SpectralBlob = SpectralBlobBase<float>;
//using SpectralBlobV = SpectralBlobBase<vfloat>;

} // namespace PR
