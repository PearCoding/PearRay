#pragma once

#include "PR_Config.h"

namespace PR {
namespace Internal {
class AutoDiff_TraitBase {
};
};

template <typename Op, typename Lhs, typename Rhs>
class AutoDiff_BinaryOp {
public:
	using return_t = typename Op::return_type<Lhs, Rhs>::type;

	inline return_t v() const { return mOp.v(mLhs, mRhs); }
	inline return_t d(size_t p) const { return mOp.d(p, mLhs, mRhs); }

	inline const Op& op() const { return mOp; }
	inline const Lhs& lhs() const { return mLhs; }
	inline const Rhs& rhs() const { return mRhs; }

private:
	Op mOp;
	Lhs mLhs;
	Rhs mRhs;
};

template <typename Derived>
class AutoDiff_Base : public Internal::AutoDiff_TraitBase {
public:
	using value_t = typename Derived::value_t;

	// Accessor
	inline const value_t& v() const;
	inline value_t& v();

	inline const value_t& d(size_t p) const;
	inline value_t& d(size_t p);

	// Copy
	inline AutoDiff_Base<Derived> clone() const;

	// Other
	template <typename U, typename dU>
	inline void applyUnaryExpr(U expr, dU dexpr);
	template <typename U, typename dU>
	inline AutoDiff_Base<Derived> unaryExpr(U expr, dU dexpr) const;

	template <typename Derived2, typename B, typename dB1, typename dB2>
	inline AutoDiff_Base<Derived> applyBinaryExpr(const AutoDiff_Base<Derived2>& other,
												  B expr, dB1 dexpr1, dB2 dexpr2);
	template <typename T2, typename B, typename dB1, typename dB2>
	inline AutoDiff_Base<Derived> binaryExpr(const AutoDiff_Base<Derived2>& other,
											 B expr, dB1 dexpr1, dB2 dexpr2) const;
};
};