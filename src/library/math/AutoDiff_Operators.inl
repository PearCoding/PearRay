namespace PR {

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Add<T1, T2>>
operator+(const T1& v1, const T2& v2)
{
	return Lazy::ADLO_Add<T1, T2>(v1, v2);
}

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Neg<T1>>
operator-(const T1& v1)
{
	return Lazy::ADLO_Neg<T1>(v1);
}

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Sub<T1, T2>>
operator-(const T1& v1, const T2& v2)
{
	return Lazy::ADLO_Sub<T1, T2>(v1, v2);
}

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Mul<T1, T2>>
operator*(const T1& v1, const T2& v2)
{
	return Lazy::ADLO_Mul<T1, T2>(v1, v2);
}

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator*(const T1& v1, typename T1::scalar_t v2)
{
	return Lazy::ADLO_Scale<T1, typename T1::scalar_t>(v1, v2);
}

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator*(typename T1::scalar_t v2, const T1& v1)
{
	return Lazy::ADLO_Scale<T1, typename T1::scalar_t>(v1, v2);
}

template <typename T1, typename T2>
inline Lazy::enable_if_adlo_op_t<T1, T2, Lazy::ADLO_Div<T1, T2>>
operator/(const T1& v1, const T2& v2)
{
	return Lazy::ADLO_Div<T1, T2>(v1, v2);
}

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_Scale<T1, typename T1::value_t>>
operator/(const T1& v1, typename T1::scalar_t v2)
{
	return Lazy::ADLO_Scale<T1, typename T1::scalar_t>(v1, 1 / v2);
}

template <typename T1>
inline Lazy::enable_if_adlo_op1_t<T1, Lazy::ADLO_InvScale<T1, typename T1::value_t>>
operator/(typename T1::scalar_t v2, const T1& v1)
{
	return Lazy::ADLO_InvScale<T1, typename T1::scalar_t>(v1, v2);
}
}