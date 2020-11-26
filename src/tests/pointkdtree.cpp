#include "container/PointKdTree.h"
#include "Random.h"
#include "Test.h"

using namespace PR;

using Map = PointKdTree<Vector3f>;
PR_BEGIN_TESTCASE(PointKdTree)
PR_TEST("Initialization")
{
	Map map(32);

	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedElements(), 0ULL);
}
PR_TEST("Store")
{
	constexpr size_t ELEMENTS = 14;
	Map map(32);

	for (uint64 k = 0; k < ELEMENTS; ++k)
		map.store(Vector3f(0, 1, 2));

	PR_CHECK_FALSE(map.isEmpty());
	PR_CHECK_EQ(map.storedElements(), ELEMENTS);

	map.reset();
	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedElements(), 0ULL);
}
PR_TEST("Search")
{
	constexpr size_t ELEMENTS = 14;

	Map map(32);
	Random random(42);

	for (uint64 k = 0; k < ELEMENTS; ++k)
		map.store(random.get3D());

	map.balanceTree();

	size_t found = 0;
	auto accum	 = [&](const Vector3f&) { ++found; };

	map.search(Vector3f(0, 0, 0), 2, accum);
	PR_CHECK_EQ(found, ELEMENTS);

	found = 0;
	map.search(Vector3f(5, 0, 0), 2, accum);
	PR_CHECK_EQ(found, 0ULL);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(PointKdTree);
PRT_END_MAIN