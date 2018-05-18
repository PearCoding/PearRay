namespace PR {
namespace Lazy {
// Add
template <typename T1, typename T2>
inline typename ADLO_BinaryBase<T1, T2>::value_t ADLO_Add<T1, T2>::v() const
{
	return this->mArg1.v() + this->mArg2.v();
}

template <typename T1, typename T2>
inline typename ADLO_BinaryBase<T1, T2>::value_t ADLO_Add<T1, T2>::d(uint32 i) const
{
	return this->mArg1.d(i) + this->mArg2.d(i);
}

// Neg
template <typename T1>
inline typename ADLO_UnaryBase<T1>::value_t ADLO_Neg<T1>::v() const
{
	return -this->mArg1.v();
}

template <typename T1>
inline typename ADLO_UnaryBase<T1>::value_t ADLO_Neg<T1>::d(uint32 i) const
{
	return -this->mArg1.d(i);
}

// Sub
template <typename T1, typename T2>
inline typename ADLO_BinaryBase<T1, T2>::value_t ADLO_Sub<T1, T2>::v() const
{
	return this->mArg1.v() - this->mArg2.v();
}

template <typename T1, typename T2>
inline typename ADLO_BinaryBase<T1, T2>::value_t ADLO_Sub<T1, T2>::d(uint32 i) const
{
	return this->mArg1.d(i) - this->mArg2.d(i);
}

// Scale
template <typename T1, typename T2>
inline typename ADLO_BinaryValueBase<T1, T2>::value_t ADLO_Scale<T1, T2>::v() const
{
	return this->mArg1.v() * this->mArg2;
}

template <typename T1, typename T2>
inline typename ADLO_BinaryValueBase<T1, T2>::value_t ADLO_Scale<T1, T2>::d(uint32 i) const
{
	return this->mArg1.d(i) * this->mArg2;
}

// Inv Scale
template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t>
ADLO_InvScale<T1, T2>::v() const
{
	return this->mArg2 / this->mArg1.v();
}
template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t>
ADLO_InvScale<T1, T2>::v() const
{
	return this->mArg2 * this->mArg1.v().cwiseInverse();
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t>
ADLO_InvScale<T1, T2>::d(uint32 i) const
{
	const auto v = 1 / (this->mArg1.v() * this->mArg1.v());
	return -this->mArg2 * this->mArg1.d(i) * v;
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryValueBase<U1, U2>::is_scalar::value, typename ADLO_BinaryValueBase<U1, U2>::value_t>
ADLO_InvScale<T1, T2>::d(uint32 i) const
{
	const auto v = this->mArg1.v().cwiseProduct(this->mArg1.v()).cwiseInverse();
	return -this->mArg2 * this->mArg1.d(i).cwiseProduct(v);
}

// Mul
template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Mul<T1, T2>::v() const
{
	return this->mArg1.v() * this->mArg2.v();
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Mul<T1, T2>::v() const
{
	return this->mArg1.v().cwiseProduct(this->mArg2.v());
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Mul<T1, T2>::d(uint32 i) const
{
	return this->mArg1.d(i) * this->mArg2.v() + this->mArg1.v() * this->mArg2.d(i);
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Mul<T1, T2>::d(uint32 i) const
{
	return this->mArg1.d(i).cwiseProduct(this->mArg2.v())
		   + this->mArg1.v().cwiseProduct(this->mArg2.d(i));
}

// Div
template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Div<T1, T2>::v() const
{
	return this->mArg1.v() / this->mArg2.v();
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Div<T1, T2>::v() const
{
	return this->mArg1.v().cwiseQuotient(this->mArg2.v());
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Div<T1, T2>::d(uint32 i) const
{
	const auto inv = 1 / (this->mArg2.v() * this->mArg2.v());
	return (this->mArg1.d(i) * this->mArg2.v() - this->mArg1.v() * this->mArg2.d(i)) * inv;
}

template <typename T1, typename T2>
template <typename U1, typename U2>
inline std::enable_if_t<!ADLO_BinaryBase<U1, U2>::is_scalar::value, typename ADLO_BinaryBase<U1, U2>::value_t>
ADLO_Div<T1, T2>::d(uint32 i) const
{
	const auto inv = this->mArg2.v().cwiseProduct(this->mArg2.v()).cwiseInverse();
	return (this->mArg1.d(i).cwiseProduct(this->mArg2.v())
			- this->mArg1.v().cwiseProduct(this->mArg2.d(i)))
		.cwiseProduct(inv);
}

// Abs
template <typename T1>
template <typename U1>
inline std::enable_if_t<ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t>
ADLO_Abs<T1>::v() const
{
	return std::abs(this->mArg1.v());
}

template <typename T1>
template <typename U1>
inline std::enable_if_t<!ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t>
ADLO_Abs<T1>::v() const
{
	return this->mArg1.v().cwiseAbs();
}

template <typename T1>
template <typename U1>
inline std::enable_if_t<ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t>
ADLO_Abs<T1>::d(uint32 i) const
{
	const auto sign = std::signbit(this->mArg1.v()) ? -1 : 1;
	return this->mArg1.d(i) * sign;
}

template <typename T1>
template <typename U1>
inline std::enable_if_t<!ADLO_UnaryBase<U1>::is_scalar::value, typename ADLO_UnaryBase<U1>::value_t>
ADLO_Abs<T1>::d(uint32 i) const
{
	const auto sign = this->mArg1.v().unaryExpr([](auto v) { return std::signbit(v) ? -1 : 1; });
	return this->mArg1.d(i).cwiseProduct(sign);
}
}
}