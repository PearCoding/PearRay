#include "container/QuickSort.h"
#include <vector>

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(QuickSort)
PR_TEST("Sorted")
{
	constexpr size_t SIZE = 1000;
	std::vector<uint32> data(SIZE);
	for (uint32 i = 0; i < (uint32)SIZE; ++i) {
		data[i] = i;
	}

	quickSort(data.data(),
			  [&](size_t a, size_t b) {
				  std::swap(data[a], data[b]);
			  },
			  0, SIZE - 1);

	bool sorted = true;
	for (uint32 i = 0; i < (uint32)SIZE; ++i) {
		if (data[i] != i) {
			sorted = false;
			break;
		}
	}

	PR_CHECK_TRUE(sorted);
}

PR_TEST("Unsorted")
{
	constexpr size_t SIZE = 1000;
	std::vector<uint32> data(SIZE);
	for (uint32 i = 0; i < (uint32)SIZE; ++i) {
		data[i] = rand();
	}

	quickSort(data.data(),
			  [&](size_t a, size_t b) {
				  std::swap(data[a], data[b]);
			  },
			  0, SIZE - 1);

	bool sorted = true;
	for (uint32 i = 0; i < (uint32)SIZE - 1; ++i) {
		if (data[i] > data[i+1]) {
			sorted = false;
			break;
		}
	}

	PR_CHECK_TRUE(sorted);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(QuickSort);
PRT_END_MAIN