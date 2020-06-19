// IWYU pragma: private, include "PR_Config.h"
namespace PR {

// a*b - c*d in a robust way
// See https://pharr.org/matt/blog/2019/11/03/difference-of-floats.html for more information
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
diffProd(T a, T b, T c, T d)
{
	T cd  = c * d;
	T err = std::fma(-c, d, cd);
	T dop = std::fma(a, b, -cd);
	return dop + err;
}

// a*b + c*d
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
sumProd(T a, T b, T c, T d)
{
	// TODO: Find a better way
	//return diffProd<T>(a, b, -c, d);
	return std::fma(a, b, c * d);
}

} // namespace PR