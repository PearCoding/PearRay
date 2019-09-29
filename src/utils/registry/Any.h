#pragma once

#include "PR_Config.h"

#include <boost/lexical_cast.hpp>
#include <exception>
#include <typeinfo>

/* A simple any class implementation.
 * Not as versatile as boost::any, but allows the use of boost::lexical_cast
 * without knowing the actual type.
 * Limited to arithmetic, enums and other special types.
 */
namespace PR {
class PR_LIB_UTILS BadCast : public std::exception {
public:
	inline explicit BadCast(const char* what)
		: mWhat(what)
	{
	}

	inline const char* what() const noexcept override { return mWhat; }

private:
	const char* mWhat;
};

#define _AH_MEMB_INV(T)                                                        \
	virtual inline void cast(T&) const { throw BadCast("Invalid void cast"); }

class PR_LIB_UTILS _AnyHolder {
public:
	_AnyHolder()		  = default;
	virtual ~_AnyHolder() = default;

	_AH_MEMB_INV(bool)
	_AH_MEMB_INV(float)
	_AH_MEMB_INV(double)
	_AH_MEMB_INV(uint8)
	_AH_MEMB_INV(int8)
	_AH_MEMB_INV(uint16)
	_AH_MEMB_INV(int16)
	_AH_MEMB_INV(uint32)
	_AH_MEMB_INV(int32)
	_AH_MEMB_INV(uint64)
	_AH_MEMB_INV(int64)
	_AH_MEMB_INV(std::string)

	template <typename T>
	inline typename std::enable_if<std::is_enum<T>::value>::type cast(T& v) const
	{
		typedef std::underlying_type_t<T> type_t;

		type_t i;
		cast(i);
		v = static_cast<T>(i);
	}

	inline virtual const std::type_info& type() const
	{
		return typeid(void);
	}

	inline virtual _AnyHolder* clone() const
	{
		return new _AnyHolder();
	}
};
#undef _AH_MEMB_INV

#define _AH_MEMB(O)                                                \
	inline void cast(O& v) const override { internal_cast<O>(v); }

template <typename T, typename = void>
class PR_LIB_UTILS _AnyHolderImpl : public _AnyHolder {
public:
	typedef std::remove_cv_t<std::decay_t<T>> value_t;

	inline explicit _AnyHolderImpl(const value_t& v)
		: _AnyHolder()
		, mValue(v)
	{
	}

	_AH_MEMB(bool)
	_AH_MEMB(float)
	_AH_MEMB(double)
	_AH_MEMB(uint8)
	_AH_MEMB(int8)
	_AH_MEMB(uint16)
	_AH_MEMB(int16)
	_AH_MEMB(uint32)
	_AH_MEMB(int32)
	_AH_MEMB(uint64)
	_AH_MEMB(int64)
	_AH_MEMB(std::string)

	inline const std::type_info& type() const override
	{
		return typeid(value_t);
	}

	inline _AnyHolder* clone() const override
	{
		return new _AnyHolderImpl<value_t>(mValue);
	}

protected:
	template <typename O>
	inline void internal_cast(O& v) const
	{
		try {
			v = boost::lexical_cast<O, value_t>(mValue);
		} catch (const boost::bad_lexical_cast&) {
			throw BadCast("Invalid lexical cast");
		}
	}

private:
	value_t mValue;
};
#undef _AH_MEMB

class PR_LIB_UTILS Any {
public:
	// Default constructor
	inline Any()
		: mInternal(new _AnyHolder())
	{
	}

	// Type constructor
	template <typename T>
	inline Any(const T& v)
		: mInternal(new _AnyHolderImpl<T>(v))
	{
	}
	template <typename T>
	inline Any& operator=(const T& v)
	{
		mInternal = new _AnyHolderImpl<T>(v);
		return *this;
	}

	// Copy
	inline Any(const Any& other)
		: mInternal(other.mInternal->clone())
	{
	}

	inline Any& operator=(const Any& other)
	{
		mInternal.reset(other.mInternal->clone());
		return *this;
	}

	// Move
	Any(Any&& other) = default;
	Any& operator=(Any&& other) = default;

	// Deconstructor
	virtual ~Any() = default;

	// Cast
	template <typename T>
	inline T cast() const
	{
		T v;
		mInternal->cast(v);
		return v;
	}

	inline const std::type_info& type() const { return mInternal->type(); }

private:
	std::unique_ptr<_AnyHolder> mInternal;
};
}
