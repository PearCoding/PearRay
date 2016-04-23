#pragma once

#include "Config.h"

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
		Test(const std::string& name) :
			mName(name)
		{
			mCount = 0;
		}

		void assert(bool cond, const std::string& msg, const std::string& func, int line)
		{
			if (!cond)
			{
				add_error(msg, func, line);
			}
			mCount++;
		}

		bool end()
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
		void add_error(const std::string& msg, const std::string& func, int line)
		{
			mErrors.push_back({ msg, func, line });
		}
	};

	class TestCase
	{
	public:
		TestCase(const std::string& name) :
			mErrors(0), mCount(0)
		{
			std::cout << "Test case: " << name << std::endl;
		}

		Test* begin(const std::string& name)
		{
			return new Test(name);
		}

		void end(Test* test)
		{
			mCount++;
			std::cout << "######################" << std::endl;
			std::cout << mCount << ". ";
			if (!test->end())
				mErrors++;
			std::cout << std::endl;

			delete test;
		}

		bool end()
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

#define PR_BEGIN_TESTCASE(name) \
	{\
	PRT::TestCase _testcase(name);\
	PRT::Test* _test = nullptr

#define PR_END_TESTCASE() \
	if (_test)\
		_testcase.end(_test);\
		if(!_testcase.end())\
			PR_DEBUG_BREAK(); \
	}

#define PR_TEST(name) \
	if (_test)\
		_testcase.end(_test);\
	_test = _testcase.begin(name)


#define PR_FUNCTION_NAME __func__


#define PR_CHECK_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected " << (expected) << " but got " << (value); \
		_test->assert((value) == (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected " << (expected) << " but got it"; \
		_test->assert((value) != (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less then " << (expected) << " but got " << (value); \
		_test->assert((value) < (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_LESS_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected less or equal then " << (expected) << " but got " << (value); \
		_test->assert((value) <= (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater then " << (expected) << " but got " << (value); \
		_test->assert((value) < (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_GREAT_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected greater or equal then " << (expected) << " but got " << (value); \
		_test->assert((value) <= (expected), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected (nearly) " << (expected) << " but got " << (value); \
		_test->assert(std::abs((value) - (expected)) <= std::numeric_limits<float>::epsilon(), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NEARLY_EQ(value, expected) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected (nearly) " << (expected) << " but got it"; \
		_test->assert(std::abs((value) - (expected)) > std::numeric_limits<float>::epsilon(), _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NULL(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Expected NULL but got it"; \
		_test->assert((value) == nullptr, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}

#define PR_CHECK_NOT_NULL(value) \
	{ \
		std::stringstream _stream; \
		_stream << "Not expected NULL but got it"; \
		_test->assert((value) != nullptr, _stream.str(), PR_FUNCTION_NAME, __LINE__); \
	}