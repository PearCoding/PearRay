#include "container/CSV.h"
#include "Test.h"

using namespace PR;

static const char* TEST_INPUT1
	= "Hello, first, row, this!\n"
	  "0, 1, 2, 3,\n"
	  "-1,0.5,0.0,0";

static const char* TEST_INPUT2
	= "0, 4, 2, 3\n"
	  "0, 1, 2, fsdfe3,\n"
	  "-1,0.5f,0.0,0";

static const char* TEST_INPUT3
	= "Hello, first, , this!\n"
	  "0, 0";

PR_BEGIN_TESTCASE(CSV)
PR_TEST("Read 1")
{
	std::stringstream stream(TEST_INPUT1, std::ios::in);
	CSV csv(stream);

	PR_CHECK_TRUE(csv.isValid());
	PR_CHECK_EQ(csv.rowCount(), 2);
	PR_CHECK_EQ(csv.columnCount(), 4);

	PR_CHECK_EQ(csv(0, 1), 1);
	PR_CHECK_EQ(csv(1, 0), -1);
	PR_CHECK_EQ(csv(1, 1), 0.5f);
}
PR_TEST("Read 2")
{
	std::stringstream stream(TEST_INPUT2, std::ios::in);
	CSV csv(stream);

	PR_CHECK_TRUE(csv.isValid());
	PR_CHECK_EQ(csv.rowCount(), 3);
	PR_CHECK_EQ(csv.columnCount(), 4);

	PR_CHECK_EQ(csv(0, 1), 4);
	PR_CHECK_EQ(csv(1, 3), 0);
	PR_CHECK_EQ(csv(2, 0), -1);
	PR_CHECK_EQ(csv(2, 1), 0.5f);
}
PR_TEST("Read 3")
{
	std::stringstream stream(TEST_INPUT3, std::ios::in);
	CSV csv(stream);

	PR_CHECK_FALSE(csv.isValid());
}
PR_TEST("Write")
{
	CSV csv(1, 1);
	csv.header(0) = "TEST";
	csv(0, 0)	  = 42;

	std::stringstream ignore;
	PR_CHECK_TRUE(csv.write(ignore));
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(CSV);
PRT_END_MAIN