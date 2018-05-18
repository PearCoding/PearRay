namespace PR {
template <typename T, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&>
AutoDiff_Scalar<T, P>::operator+=(const U& other)
{
	this->mValue += other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] += other.d(i);
	}
	return *this;
}

template <typename T, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&>
AutoDiff_Scalar<T, P>::operator-=(const U& other)
{
	this->mValue -= other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] -= other.d(i);
	}
	return *this;
}

template <typename T, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&>
AutoDiff_Scalar<T, P>::operator*=(const U& other)
{
	const auto v = other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] = other.d(i) * this->mValue + this->mGradients[i] * v;
	}

	this->mValue *= v;
	return *this;
}

template <typename T, size_t P>
inline AutoDiff_Scalar<T, P>&
AutoDiff_Scalar<T, P>::operator*=(const value_t& other)
{
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] *= other;
	}

	this->mValue *= other;
	return *this;
}

template <typename T, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_scalar1_t<U, AutoDiff_Scalar<T, P>&>
AutoDiff_Scalar<T, P>::operator/=(const U& other)
{
	const auto v	  = other.v();
	const value_t inv = 1 / (v * v);

	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] = (-other.d(i) * this->mValue + this->mGradients[i] * v) * inv;
	}

	this->mValue /= v;

	return *this;
}

template <typename T, size_t P>
inline AutoDiff_Scalar<T, P>&
AutoDiff_Scalar<T, P>::operator/=(const value_t& other)
{
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] /= other;
	}

	this->mValue /= other;

	return *this;
}

template <typename T, size_t P>
inline typename AutoDiff_Scalar<T, P>::InvReturnType
AutoDiff_Scalar<T, P>::inv() const
{
	return InvReturnType(*this, 1);
}

template <typename T, size_t P>
inline typename AutoDiff_Scalar<T, P>::AbsReturnType
AutoDiff_Scalar<T, P>::abs() const
{
	return AbsReturnType(*this);
}

template <typename T, size_t P>
template <typename U, typename dU>
inline void AutoDiff_Scalar<T, P>::applyUnaryExpr(U expr, dU dexpr)
{
	const auto v = dexpr(this->mValue);
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] *= v;
	}
	this->mValue = expr(this->mValue);
}
template <typename T, size_t P>
template <typename U, typename dU>
inline typename AutoDiff_Scalar<T, P>::self_t
AutoDiff_Scalar<T, P>::unaryExpr(U expr, dU dexpr) const
{
	self_t r = *this;
	r.applyUnaryExpr(expr, dexpr);
	return r;
}

template <typename T, size_t P>
template <typename T2, typename B, typename dB1, typename dB2>
inline Lazy::enable_if_adlo_scalar1_t<T2, void> AutoDiff_Scalar<T, P>::applyBinaryExpr(const T2& other, B expr, dB1 dexpr1, dB2 dexpr2)
{
	const auto v1 = dexpr1(this->mValue, other.v());
	const auto v2 = dexpr2(this->mValue, other.v());

	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] = this->mGradients[i] * v1 + other.d(i) * v2;
	}
	this->mValue = expr(this->mValue, other.v());
}

template <typename T, size_t P>
template <typename T2, typename B, typename dB1, typename dB2>
inline Lazy::enable_if_adlo_scalar1_t<T2, typename AutoDiff_Scalar<T, P>::self_t>
AutoDiff_Scalar<T, P>::binaryExpr(const T2& other, B expr, dB1 dexpr1, dB2 dexpr2) const
{
	self_t r = *this;
	r.applyBinaryExpr(other, expr, dexpr1, dexpr2);
	return r;
}

// Useful functions
template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> sqrt(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::sqrt(v); },
											 [](typename T::value_t v) { return 1 / (2 * std::sqrt(v)); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> cbrt(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::cbrt(v); },
											 [](typename T::value_t v) { return 1 / (3 * std::pow(std::cbrt(v), 2)); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> sin(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::sin(v); },
											 [](typename T::value_t v) { return std::cos(v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> asin(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::asin(v); },
											 [](typename T::value_t v) { return 1 / std::sqrt(1 - v * v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> cos(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::cos(v); },
											 [](typename T::value_t v) { return -std::sin(v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> acos(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::acos(v); },
											 [](typename T::value_t v) { return -1 / std::sqrt(1 - v * v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> tan(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::tan(v); },
											 [](typename T::value_t v) { return 1 / std::pow(std::cos(v), 2); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> atan(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::atan(v); },
											 [](typename T::value_t v) { return 1 / (1 + v * v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> exp(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::exp(v); },
											 [](typename T::value_t v) { return std::exp(v); });
}

template <typename T>
inline Lazy::enable_if_adlo_scalar1_t<T, typename T::self_t> log(const T& s)
{
	return (typename T::self_t)(s).unaryExpr([](typename T::value_t v) { return std::log(v); },
											 [](typename T::value_t v) { return 1 / v; });
}
}