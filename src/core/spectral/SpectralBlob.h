#pragma once

#include "PR_Config.h"

namespace PR {

constexpr size_t PR_SPECTRAL_BLOB_SIZE = 4;

template <typename T>
using SpectralBlobBase = Eigen::Array<T, PR_SPECTRAL_BLOB_SIZE, 1>;

template <typename T>
using SpectralBlobStorageBase = Eigen::Array<T, PR_SPECTRAL_BLOB_SIZE, 1, Eigen::DontAlign>;

using SpectralBlob		  = SpectralBlobBase<float>;
using SpectralBlobStorage = SpectralBlobStorageBase<float>;

namespace SpectralBlobUtils {
inline auto HeroOnly() { return SpectralBlob(1, 0, 0, 0); }
} // namespace SpectralBlobUtils
} // namespace PR
