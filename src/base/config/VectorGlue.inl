// IWYU pragma: private, include "PR_Config.h"
namespace PR {

/* Variadic vector types */
template <typename T>
using Vector2t = Eigen::Matrix<T, 2, 1>;
template <typename T>
using Vector3t = Eigen::Matrix<T, 3, 1>;
template <typename T>
using Vector4t = Eigen::Matrix<T, 4, 1>;

/* Precise vector types */
using Vector2f	= Eigen::Vector2f;
using Vector2i	= Eigen::Vector2i;

using Vector3f	= Eigen::Vector3f;
using Vector3i	= Eigen::Vector3i;

using Vector4f	= Eigen::Vector4f;
using Vector4i	= Eigen::Vector4i;

} // namespace PR