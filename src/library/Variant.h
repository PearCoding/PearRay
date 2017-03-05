#pragma once

#include "PR_Config.h"

namespace PR
{
	/* Simple Variant type. */
	class PR_LIB_INLINE Variant
	{
	public:
		enum Type
		{
			T_Bool = 0,
			T_Int = 1,
			T_UInt = 2,
			T_Number = 3,
			T_String = 4
		};

		inline Variant() : mType(T_Bool), mBool(false) {}
		inline Variant(bool b) : mType(T_Bool), mBool(b) {}
		inline Variant(int64 v) : mType(T_Int), mInt(v) {}
		inline Variant(uint64 v) : mType(T_UInt), mUInt(v) {}
		inline Variant(double v) : mType(T_Number), mNumber(v) {}
		inline Variant(const std::string& v) : mType(T_String), mString(v) {}

		inline Type type() const { return mType; }
		inline void set(bool b) { mType = T_Bool; mBool = b; }
		inline void set(int64 v) { mType = T_Int; mInt = v; }
		inline void set(uint64 v) { mType = T_UInt; mUInt = v; }
		inline void set(double v) { mType = T_Number; mNumber = v; }
		inline void set(const std::string& v) { mType = T_String; mString = v; }

		inline bool getBool() const { return mBool; }
		inline int64 getInt() const { return mInt; }
		inline uint64 getUInt() const { return mUInt; }
		inline double getNumber() const { return mNumber; }
		inline const std::string& getString() const { return mString; }

	private:
		Type mType;

		union {
			bool mBool;
			int64 mInt;
			uint64 mUInt;
			double mNumber;
		};
		std::string mString;
	};
}
