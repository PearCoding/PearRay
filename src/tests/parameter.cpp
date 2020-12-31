#include "parameter/Parameter.h"
#include "parameter/ParameterGroup.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Parameter)
PR_TEST("Parameter [Invalid]")
{
	Parameter parameter;
	PR_CHECK_FALSE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::Invalid);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 0);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(42), 42);
}
PR_TEST("Parameter [Bool]")
{
	Parameter parameter = Parameter::fromBool(true);
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::Bool);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 1);
	PR_CHECK_TRUE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(42), 42);
}
PR_TEST("Parameter [Int]")
{
	Parameter parameter = Parameter::fromInt(-42);
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::Int);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 1);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(0), -42);
	PR_CHECK_EQ(parameter.getUInt(0), 0);
	PR_CHECK_EQ(parameter.getExactUInt(0), 0);
}
PR_TEST("Parameter [UInt]")
{
	Parameter parameter = Parameter::fromUInt(42);
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::UInt);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 1);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(0), 42);
	PR_CHECK_EQ(parameter.getExactInt(0), 0);
	PR_CHECK_EQ(parameter.getUInt(0), 42);
}
PR_TEST("Parameter [Number]")
{
	Parameter parameter = Parameter::fromNumber(PR_PI);
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::Number);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 1);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(0), static_cast<int64>(PR_PI));
	PR_CHECK_EQ(parameter.getExactInt(0), 0);
	PR_CHECK_EQ(parameter.getNumber(0), PR_PI);
}
PR_TEST("Parameter [String]")
{
	const std::string test = "adrian";
	Parameter parameter	   = Parameter::fromString(test);
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::String);
	PR_CHECK_FALSE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 1);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(0), 0);
	PR_CHECK_EQ(parameter.getExactInt(0), 0);
	PR_CHECK_EQ(parameter.getString(""), test);
}
PR_TEST("Parameter Array[Int]")
{
	Parameter parameter = Parameter::fromIntArray({ 0, 1, 2, 3 });
	PR_CHECK_TRUE(parameter.isValid());
	PR_CHECK_EQ(parameter.type(), ParameterType::Int);
	PR_CHECK_TRUE(parameter.isArray());
	PR_CHECK_EQ(parameter.arraySize(), 4);
	PR_CHECK_FALSE(parameter.getBool(false));
	PR_CHECK_EQ(parameter.getInt(2, 0), 2);
	PR_CHECK_EQ(parameter.getUInt(2, 0), 2);
}
PR_TEST("ParameterGroup")
{
	ParameterGroup group;
	group.addParameter("nethan", Parameter::fromInt(42));
	group.addParameter("ria", Parameter::fromBool(true));
	group.addParameter("elia", Parameter::fromNumber(PR_PI));
	PR_CHECK_TRUE(group.hasParameter("nethan"));
	PR_CHECK_FALSE(group.hasParameter("bendis"));

	PR_CHECK_EQ(group.getInt("ria", 0), 0);
	PR_CHECK_EQ(group.getUInt("nethan", 0), 42);
	PR_CHECK_EQ(group.getNumber("elia", 0), PR_PI);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Parameter);
PRT_END_MAIN