#pragma once

#include "Config.h"
#include "PearMath.h"

#include <iostream>
#include <list>
#include <limits>
#include <string>
#include <sstream>

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
			{
				add_error(msg, func, line);
			}
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
	_test = _testcase.begin(name)


#define PR_CHECK_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected " << (expected) << " but got " << (value); \
		_test->check((value) == (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_EQ_2(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << "]"; \
		_test->check(PM::pm_GetX((value)) == PM::pm_GetX((expected)) && PM::pm_GetY((value)) == PM::pm_GetY((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_EQ_3(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << ", " << PM::pm_GetZ((value)) << "]"; \
		_test->check(PM::pm_GetX((value)) == PM::pm_GetX((expected)) && PM::pm_GetY((value)) == PM::pm_GetY((expected)) && PM::pm_GetZ((value)) == PM::pm_GetZ((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_EQ_4(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << ", " << PM::pm_GetW((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << ", " << PM::pm_GetZ((value)) << ", " << PM::pm_GetW((value)) << "]"; \
		_test->check(PM::pm_GetX((value)) == PM::pm_GetX((expected)) && PM::pm_GetY((value)) == PM::pm_GetY((expected)) && PM::pm_GetZ((value)) == PM::pm_GetZ((expected)) && PM::pm_GetW((value)) == PM::pm_GetW((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected " << (expected) << " but got it"; \
		_test->check((value) != (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ_2(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << "] but got it"; \
		_test->check(PM::pm_GetX((value)) != PM::pm_GetX((expected)) || PM::pm_GetY((value)) != PM::pm_GetY((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ_3(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << "] but got it"; \
		_test->check(PM::pm_GetX((value)) != PM::pm_GetX((expected)) || PM::pm_GetY((value)) != PM::pm_GetY((expected)) || PM::pm_GetZ((value)) != PM::pm_GetZ((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ_4(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << ", " << PM::pm_GetW((expected)) << "] but got it"; \
		_test->check(PM::pm_GetX((value)) != PM::pm_GetX((expected)) || PM::pm_GetY((value)) != PM::pm_GetY((expected)) || PM::pm_GetZ((value)) != PM::pm_GetZ((expected)) || PM::pm_GetW((value)) != PM::pm_GetW((expected)), \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less then " << (expected) << " but got " << (value); \
		_test->check((value) < (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less or equal then " << (expected) << " but got " << (value); \
		_test->check((value) <= (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater then " << (expected) << " but got " << (value); \
		_test->check((value) > (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater or equal then " << (expected) << " but got " << (value); \
		_test->check((value) >= (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) " << (expected) << " but got " << (value); \
		_test->check(std::abs((value) - (expected)) <= PRT_EPSILON, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ_2(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << "]"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) <= PRT_EPSILON && \
		 std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) <= PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ_3(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << ", " << PM::pm_GetZ((value)) << "]"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) <= PRT_EPSILON && \
		 std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) <= PRT_EPSILON && \
		std::abs(PM::pm_GetZ((value)) - PM::pm_GetZ((expected))) <= PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ_4(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << ", " << PM::pm_GetW((expected)) << "] but got [" << PM::pm_GetX((value)) << ", " << PM::pm_GetY((value)) << ", " << PM::pm_GetZ((value)) << ", " << PM::pm_GetW((value)) << "]"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) <= PRT_EPSILON && \
		std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) <= PRT_EPSILON && \
		std::abs(PM::pm_GetZ((value)) - PM::pm_GetZ((expected))) <= PRT_EPSILON && \
		std::abs(PM::pm_GetW((value)) - PM::pm_GetW((expected))) <= PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) " << (expected) << " but got it"; \
		_test->check(std::abs((value) - (expected)) > std::numeric_limits<float>::epsilon(), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ_2(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << "] but got it"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) > PRT_EPSILON || \
		 std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) > PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ_3(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << "] but got it"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) <= PRT_EPSILON || \
		 std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) > PRT_EPSILON || \
		std::abs(PM::pm_GetZ((value)) - PM::pm_GetZ((expected))) > PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ_4(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) [" << PM::pm_GetX((expected)) << ", " << PM::pm_GetY((expected)) << ", " << PM::pm_GetZ((expected)) << ", " << PM::pm_GetW((expected)) << "] but got it"; \
		_test->check(std::abs(PM::pm_GetX((value)) - PM::pm_GetX((expected))) > PRT_EPSILON || \
		std::abs(PM::pm_GetY((value)) - PM::pm_GetY((expected))) > PRT_EPSILON || \
		std::abs(PM::pm_GetZ((value)) - PM::pm_GetZ((expected))) > PRT_EPSILON || \
		std::abs(PM::pm_GetW((value)) - PM::pm_GetW((expected))) > PRT_EPSILON, \
		_stream.str(), PR_FUNCTION_NAME, __LINE__); \
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
		PR_DEBUG_ASSERT(_errors == 0); \
		return -_errors; \
	}
