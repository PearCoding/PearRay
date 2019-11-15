#include "container/RadixSort.h"
#include <vector>

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(RadixSort)
PR_TEST("Sorted")
{
	constexpr size_t SIZE = 1000;
	std::vector<uint32> data(SIZE);
	for (uint32 i = 0; i < (uint32)SIZE; ++i) {
		data[i] = i;
	}

	radixSort(data.data(),
			  [&](size_t a, size_t b) {
				  std::swap(data[a], data[b]);
			  },
			  0, SIZE - 1, (uint32)(1 << 31));

	bool sorted = true;
	for (size_t i = 0; i < SIZE; ++i) {
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
	for (size_t i = 0; i < SIZE; ++i) {
		data[i] = rand();
	}

	radixSort(data.data(),
			  [&](size_t a, size_t b) {
				  std::swap(data[a], data[b]);
			  },
			  0, SIZE - 1, (uint32)(1 << 31));

	bool sorted = true;
	for (size_t i = 0; i < SIZE-1; ++i) {
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
PRT_TESTCASE(RadixSort);
PRT_END_MAIN