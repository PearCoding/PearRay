#pragma once

#include "PR_Config.h"

#include <exception>
#include <sstream>
#include <typeinfo>
#include <vector>

#define _PR_ANY_ARRAY_DELIMITER L" "

/* A simple any class implementation.
 * Not as versatile as boost::any
 * Limited to arithmetic, enums and other special types.
 */
namespace PR {
#define _CAST_S(type)                                              \
	inline void fromString(const std::wstring& str, type& v) const \
	{                                                              \
		std::wstringstream stream(str);                            \
		stream >> v;                                               \
	}

#define _STR_S(type)                                  \
	inline std::wstring toString(const type& v) const \
	{                                                 \
		std::wstringstream stream;                    \
		stream << v;                                  \
		return stream.str();                          \
	}

#define _CAST_IS(type)                                             \
	inline void fromString(const std::wstring& str, type& v) const \
	{                                                              \
		int64 kv;                                                  \
		fromString(str, kv);                                       \
		v = static_cast<type>(kv);                                 \
	}

#define _STR_IS(type)                                 \
	inline std::wstring toString(const type& v) const \
	{                                                 \
		std::wstringstream stream;                    \
		stream << (int64)v;                           \
		return stream.str();                          \
	}

#define _CAST_US(type)                                             \
	inline void fromString(const std::wstring& str, type& v) const \
	{                                                              \
		uint64 kv;                                                 \
		fromString(str, kv);                                       \
		v = static_cast<type>(kv);                                 \
	}

#define _STR_US(type)                                 \
	inline std::wstring toString(const type& v) const \
	{                                                 \
		std::wstringstream stream;                    \
		stream << (uint64)v;                          \
		return stream.str();                          \
	}

class PR_LIB_UTILS Any {
public:
	inline Any()
		: mValue()
		, mTypeName(typeid(void).name())
	{
	}

	template <typename T>
	inline Any(const T& v)
		: mValue(toString(v))
		, mTypeName(typeid(T).name())
	{
	}

	template <typename T>
	inline Any& operator=(const T& v)
	{
		*this = Any(v);
		return *this;
	}

	// Copy
	Any(const Any& other) = default;
	Any& operator=(const Any& other) = default;

	// Move
	Any(Any&& other) = default;
	Any& operator=(Any&& other) = default;

	// Deconstructor
	virtual ~Any() = default;

	template <typename T>
	inline T cast() const
	{
		T v;
		fromString(mValue, v);
		return v;
	}

	inline const char* typeName() const { return mTypeName; }

protected:
	_CAST_S(float)
	_CAST_S(double)
	_CAST_IS(int8)
	_CAST_US(uint8)
	_CAST_IS(int16)
	_CAST_US(uint16)
	_CAST_IS(int32)
	_CAST_US(uint32)
	_CAST_S(int64)
	_CAST_S(uint64)
	_CAST_S(bool)

	template <typename T>
	inline typename std::enable_if<std::is_enum<T>::value, void>::type
	fromString(const std::wstring& str, T& v) const
	{
		uint64 k;
		fromString(str, k);
		v = (T)k;
	}

	inline void fromString(const std::wstring& str, std::wstring& v) const { v = str; }
	inline void fromString(const std::wstring& str, std::string& v) const { v = std::string(str.begin(), str.end()); }

	template <typename T>
	inline void fromString(const std::wstring& str, std::vector<T>& v) const
	{
		size_t last = 0;
		size_t next = 0;
		while ((next = str.find(_PR_ANY_ARRAY_DELIMITER, last)) != std::wstring::npos) {
			T tmp;
			fromString(str.substr(last, next - last), tmp);
			v.push_back(tmp);
			last = next + 1;
		}

		std::wstring l = str.substr(last);
		if (!l.empty()) {
			T tmp;
			fromString(l, tmp);
			v.push_back(tmp);
		}
	}

	template <typename T, int R, int C>
	inline void fromString(const std::wstring& str, Eigen::Matrix<T, R, C>& v) const
	{
		std::vector<T> arr;
		fromString(str, arr);
		if (arr.size() == R * C)
			for (int i = 0; i < R; ++i)
				for (int j = 0; j < C; ++j)
					v(i, j) = arr.at(i * C + j);
	}

	////////////////////

	_STR_S(float)
	_STR_S(double)
	_STR_S(int8)
	_STR_S(uint8)
	_STR_S(int16)
	_STR_S(uint16)
	_STR_S(int32)
	_STR_S(uint32)
	_STR_S(int64)
	_STR_S(uint64)
	_STR_S(bool)

	template <typename T>
	inline typename std::enable_if<std::is_enum<T>::value, std::wstring>::type
	toString(const T& v) const { return toString((uint64)v); }

	inline std::wstring toString(const std::wstring& v) const { return v; }
	inline std::wstring toString(const std::string& v) const { return std::wstring(v.begin(), v.end()); }

	template <typename T>
	inline std::wstring toString(const std::vector<T>& v) const
	{
		std::wstring str;
		for (const T& e : v) {
			str += toString(e) + _PR_ANY_ARRAY_DELIMITER;
		}

		return str;
	}

	template <typename T, int R, int C>
	inline std::wstring toString(const std::string& str, Eigen::Matrix<T, R, C>& v) const
	{
		std::vector<T> arr(R * C);
		for (int i = 0; i < R; ++i)
			for (int j = 0; j < C; ++j)
				arr[i * C + j] = v(i, j);

		return toString(str, arr);
	}

private:
	std::wstring mValue;
	const char* mTypeName;
};

#undef _CAST_S
#undef _STR_S
#undef _CAST_IS
#undef _STR_IS
#undef _CAST_US
#undef _STR_US
} // namespace PR
