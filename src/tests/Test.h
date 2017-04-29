#pragma once

#include "PR_Config.h"
#include "PearMath.h"

#include <iostream>
#include <list>
#include <limits>
#include <string>
#include <sstream>

/* General interface */
template<typename T1, typename T2>
bool _prt_test_eq(const T1& val, const T2& exp)
{
	return val == exp;
}

template<typename T1, typename T2>
bool _prt_test_greater(const T1& val, const T2& exp)
{
	return val > exp;
}

template<typename T1, typename T2>
bool _prt_test_diff(const T1& val, const T2& exp)
{
	return std::abs(val - exp);
}

template<typename T>
std::string _prt_test_string(const T& val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

/* 2D */
template<>
bool _prt_test_eq<PM::vec2,PM::vec2>(const PM::vec2& val, const PM::vec2& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
bool _prt_test_greater<PM::vec2,PM::vec2>(const PM::vec2& val, const PM::vec2& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsGreater(val, exp));
}

template<>
bool _prt_test_diff<PM::vec2,PM::vec2>(const PM::vec2& val, const PM::vec2& exp)
{
	return PM::pm_MagnitudeSqr(PM::pm_Subtract(val, exp));
}

/* 3D */
template<>
bool _prt_test_eq<PM::vec3,PM::vec3>(const PM::vec3& val, const PM::vec3& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
bool _prt_test_greater<PM::vec3,PM::vec3>(const PM::vec3& val, const PM::vec3& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsGreater(val, exp));
}

template<>
bool _prt_test_diff<PM::vec3,PM::vec3>(const PM::vec3& val, const PM::vec3& exp)
{
	return PM::pm_MagnitudeSqr(PM::pm_Subtract(val, exp));
}

template<>
std::string _prt_test_string<PM::vec3>(const PM::vec3& val)
{
	std::stringstream stream;
	stream << "(" << PM::pm_GetX(val) << "," << PM::pm_GetY(val) << "," << PM::pm_GetZ(val) << ")";
	return stream.str();
}

/* 4D */
template<>
bool _prt_test_eq<PM::vec4,PM::vec4>(const PM::vec4& val, const PM::vec4& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
bool _prt_test_greater<PM::vec4,PM::vec4>(const PM::vec4& val, const PM::vec4& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsGreater(val, exp));
}

template<>
bool _prt_test_diff<PM::vec4,PM::vec4>(const PM::vec4& val, const PM::vec4& exp)
{
	return PM::pm_MagnitudeSqr(PM::pm_Subtract(val, exp));
}

template<>
std::string _prt_test_string<PM::vec4>(const PM::vec4& val)
{
	std::stringstream stream;
	stream << "(" << PM::pm_GetX(val) << "," << PM::pm_GetY(val) 
		<< "," << PM::pm_GetZ(val) << "," << PM::pm_GetW(val) << ")";
	return stream.str();
}

/* mat2 */
template<>
bool _prt_test_eq<PM::mat2,PM::mat2>(const PM::mat2& val, const PM::mat2& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
std::string _prt_test_string<PM::mat2>(const PM::mat2& val)
{
	std::stringstream stream;
	stream << "\n[" << PM::pm_GetIndex(val,0,0) << "," << PM::pm_GetIndex(val,0,1)
		<< ";\n " << PM::pm_GetIndex(val,1,0) << "," << PM::pm_GetIndex(val,1,1) << "]";
	return stream.str();
}

/* mat3 */
template<>
bool _prt_test_eq<PM::mat3,PM::mat3>(const PM::mat3& val, const PM::mat3& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
std::string _prt_test_string<PM::mat3>(const PM::mat3& val)
{
	std::stringstream stream;
	stream << "\n[" << PM::pm_GetIndex(val,0,0) << "," << PM::pm_GetIndex(val,0,1) << "," << PM::pm_GetIndex(val,0,2)
		<< ";\n " << PM::pm_GetIndex(val,1,0) << "," << PM::pm_GetIndex(val,1,1) << "," << PM::pm_GetIndex(val,1,2)
		<< ";\n " << PM::pm_GetIndex(val,2,0) << "," << PM::pm_GetIndex(val,2,1) << "," << PM::pm_GetIndex(val,2,2) << "]";
	return stream.str();
}

/* mat4 */
template<>
bool _prt_test_eq<PM::mat4,PM::mat4>(const PM::mat4& val, const PM::mat4& exp)
{
	return PM::pm_IsAllTrue(PM::pm_IsEqual(val,exp));
}

template<>
std::string _prt_test_string<PM::mat4>(const PM::mat4& val)
{
	std::stringstream stream;
	stream << "\n[" << PM::pm_GetIndex(val,0,0) << "," << PM::pm_GetIndex(val,0,1) << "," << PM::pm_GetIndex(val,0,2) << "," << PM::pm_GetIndex(val,0,3)
		<< ";\n " << PM::pm_GetIndex(val,1,0) << "," << PM::pm_GetIndex(val,1,1) << "," << PM::pm_GetIndex(val,1,2) << "," << PM::pm_GetIndex(val,1,3)
		<< ";\n " << PM::pm_GetIndex(val,2,0) << "," << PM::pm_GetIndex(val,2,1) << "," << PM::pm_GetIndex(val,2,2) << "," << PM::pm_GetIndex(val,2,3)
		<< ";\n " << PM::pm_GetIndex(val,3,0) << "," << PM::pm_GetIndex(val,3,1) << "," << PM::pm_GetIndex(val,3,2) << "," << PM::pm_GetIndex(val,3,3) << "]";
	return stream.str();
}

/* Setup system */
namespace PRT
{
	class Test
	{
	private:
		struct Entry
		{
			std::string MSG;
			std::string FUNCTION;
			int LINE;
		};

		std::list<Entry> mErrors;
		int mCount;
		std::string mName;

	public:
		inline explicit Test(const std::string& name) :
			mName(name)
		{
			mCount = 0;
		}

		inline void check(bool cond, const std::string& msg, const std::string& func, int line)
		{
			if (!cond)
				add_error(msg, func, line);
			mCount++;
		}

		inline bool end() const 
		{
			std::cout << "Test '" << mName << "':" << std::endl;
			int i = 1;
			for(Entry e : mErrors)
			{
				std::cout << "  #" << i << " (" << e.FUNCTION << ": " << e.LINE << ")" << std::endl
					<< "    " << e.MSG << std::endl;
				i++;
			}
			std::cout << "----------------------" << std::endl;
			std::cout << "  " << (mCount - mErrors.size()) << "/" << mCount << " checks successful." << std::endl;

			return mErrors.size() == 0;
		}
	private:
		inline void add_error(const std::string& msg, const std::string& func, int line)
		{
			mErrors.push_back({ msg, func, line });
		}
	};

	class TestCase
	{
	public:
		inline explicit TestCase(const std::string& name) :
			mCount(0), mErrors(0)
		{
			std::cout << "Test case: " << name << std::endl;
		}

		inline Test* begin(const std::string& name)
		{
			return new Test(name);
		}

		inline void end(Test* test)
		{
			mCount++;
			std::cout << "######################" << std::endl;
			std::cout << mCount << ". ";
			if (!test->end())
				mErrors++;
			std::cout << std::endl;

			delete test;
		}

		inline bool end()
		{
			std::cout << "######################" << std::endl;
			std::cout << "Result: " << (mCount - mErrors) << "/" << mCount << " successful." << std::endl;

			return mErrors == 0;
		}

	private:
		int mCount;
		int mErrors;
	};
}

#ifndef PRT_EPSILON
# define PRT_EPSILON (PM_EPSILON * 2)
#endif

#define PR_BEGIN_TESTCASE(name) \
	int _testcase_ ##name() { \
	PRT::TestCase _testcase(PR_DOUBLEQUOTE(name)); \
	PRT::Test* _test = nullptr;

#define PR_END_TESTCASE() \
	if (_test) \
		_testcase.end(_test); \
	return _testcase.end(); \
	}

#define PR_TESTCASE(name) \
	_testcase_ ##name()

#define PR_TEST(name) \
	if (_test)\
		_testcase.end(_test);\
	_test = _testcase.begin(name);


#define PR_CHECK_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected " << _prt_test_string((expected)) << " but got it"; \
		_test->check(!_prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less then " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(!_prt_test_greater((value), (expected)) && !_prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less or equal then " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(!_prt_test_greater((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater then " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_greater((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater or equal then " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_greater((value), (expected)) || _prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_diff((value), (expected)) <= PRT_EPSILON, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) " << _prt_test_string((expected)) << " but got it"; \
		_test->check(_prt_test_diff((value), (expected)) > PRT_EPSILON, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NULL(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected NULL but not got it"; \
		_test->check((value) == nullptr, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NULL(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected NULL but got it"; \
		_test->check((value) != nullptr, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_TRUE(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected true but got false"; \
		_test->check((value) == true, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_FALSE(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected false but got true"; \
		_test->check((value) == false, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PRT_BEGIN_MAIN \
	int main(int argc, char** argv) \
	{ \
		int _errors = 0;

#define PRT_TESTCASE(name) \
		_errors += PR_TESTCASE(name) ? 0 : 1

#define PRT_END_MAIN \
		return -_errors; \
	}
