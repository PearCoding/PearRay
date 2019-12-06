#pragma once

#include "MatrixFormat.h"

#include <iostream>
#include <list>
#include <limits>
#include <string>
#include <sstream>

/* General interface */
template<typename T1, typename T2>
bool _prt_test_eq(const T1& v1, const T2& v2)
{
	return v1 == (T1)v2;
}

template<typename T1, typename T2>
bool _prt_test_greater(const T1& v1, const T2& v2)
{
	return v1 > (T1)v2;
}

template<typename T1, typename T2>
double _prt_diff(const T1& v1, const T2& v2)
{
	return std::abs(v1 - v2);
}

template<typename T>
std::string _prt_test_string(const T& val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

template<typename T1, typename T2>
bool _prt_test_diff(const T1& v1, const T2& v2)
{
	return _prt_diff(v1, v2) == 0;
}

template<typename T1, typename T2>
bool _prt_test_nearly(const T1& v1, const T2& v2, double eps)
{
	return _prt_diff(v1, v2) <= std::abs(eps);
}

// Generic
template<typename Derived>
bool _prt_test_eq_V(const Eigen::MatrixBase<Derived>& val, const Eigen::MatrixBase<Derived>& exp)
{
	return val==exp;
}

template<typename Derived>
bool _prt_test_greater_V(const Eigen::MatrixBase<Derived>& val, const Eigen::MatrixBase<Derived>& exp)
{
	return (val.array()>exp.array()).all();
}

template<typename Derived>
double _prt_diff_V(const Eigen::MatrixBase<Derived>& val, const Eigen::MatrixBase<Derived>& exp)
{
	return (val-exp).squaredNorm();
}

template<typename Derived>
std::string _prt_test_string_V(const Eigen::MatrixBase<Derived>& val)
{
	return PR_FMT_MAT(val);
}

#define _PRT_DEF_STRUCT(M) \
template<> \
bool _prt_test_eq<M,M>(const M& val, const M& exp) { return _prt_test_eq_V(val, exp); } \
template<> \
bool _prt_test_greater<M,M>(const M& val, const M& exp) { return _prt_test_greater_V(val, exp); } \
template<> \
double _prt_diff<M,M>(const M& val, const M& exp) { return _prt_diff_V(val, exp); } \
template<> \
std::string _prt_test_string<M>(const M& val) { return _prt_test_string_V(val); }

_PRT_DEF_STRUCT(Eigen::Matrix2f)
_PRT_DEF_STRUCT(Eigen::Matrix3f)
_PRT_DEF_STRUCT(Eigen::Matrix4f)
_PRT_DEF_STRUCT(PR::Vector2f)
_PRT_DEF_STRUCT(PR::Vector3f)
_PRT_DEF_STRUCT(PR::Vector4f)

#undef _PRT_DEF_STRUCT

template<>
bool _prt_test_eq(const Eigen::Quaternionf& val, const Eigen::Quaternionf& exp)
{
	return val.vec()==exp.vec();
}

template<>
bool _prt_test_greater(const Eigen::Quaternionf& val, const Eigen::Quaternionf& exp)
{
	return (val.vec().array()>exp.vec().array()).all();
}

template<>
double _prt_diff(const Eigen::Quaternionf& val, const Eigen::Quaternionf& exp)
{
	return (val.vec()-exp.vec()).squaredNorm();
}

template<>
std::string _prt_test_string(const Eigen::Quaternionf& val)
{
	std::stringstream stream;
	stream << val.vec();
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
# define PRT_EPSILON (PR_EPSILON * 2)
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
		_stream << "Expected less than " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(!_prt_test_greater((value), (expected)) && !_prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less or equal than " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(!_prt_test_greater((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater than " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_greater((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater or equal than " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_greater((value), (expected)) || _prt_test_eq((value), (expected)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_nearly((value), (expected), PRT_EPSILON), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ_EPS(value, expected, eps) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) " << _prt_test_string((expected)) << " but got " << _prt_test_string((value)); \
		_test->check(_prt_test_nearly((value), (expected), (eps)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) " << _prt_test_string((expected)) << " but got it"; \
		_test->check(!_prt_test_nearly((value), (expected), PRT_EPSILON), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ_EPS(value, expected, eps) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) " << _prt_test_string((expected)) << " but got it"; \
		_test->check(!_prt_test_nearly((value), (expected), (eps)), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
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
	int main(int /*argc*/, char** /*argv*/) \
	{ \
		int _errors = 0;

#define PRT_TESTCASE(name) \
		_errors += PR_TESTCASE(name) ? 0 : 1

#define PRT_END_MAIN \
		return -_errors; \
	}
