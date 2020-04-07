#include "memory/MemoryPool.h"
#include "memory/MemoryStack.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(MemoryPool)
PR_TEST("Allocate")
{
	MemoryPool pool(sizeof(uint32), alignof(uint32), 64);

	PR_CHECK_EQ(pool.usedMemory(), 0);
	uint32* value = pool.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_EQ(pool.usedMemory(), pool.blockSize());
	pool.freeAll();
	PR_CHECK_EQ(pool.usedMemory(), 0);
}
PR_TEST("Deallocate")
{
	MemoryPool pool(sizeof(uint32), alignof(uint32), 64);

	PR_CHECK_EQ(pool.usedMemory(), 0);
	uint32* value = pool.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_EQ(pool.usedMemory(), pool.blockSize());
	pool.destroy(value);
	value = nullptr;
	PR_CHECK_EQ(pool.usedMemory(), 0);
	value = pool.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_EQ(pool.usedMemory(), pool.blockSize());
	pool.destroy(value);
	PR_CHECK_EQ(pool.usedMemory(), 0);
}
PR_TEST("Full")
{
	MemoryPool pool(sizeof(uint32), alignof(uint32), 64);

	PR_CHECK_TRUE(pool.isEmpty());
	PR_CHECK_FALSE(pool.isFull());
	uint32* value = pool.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_FALSE(pool.isEmpty());
	PR_CHECK_FALSE(pool.isFull());
	for (int i = 0; i < 63; ++i) {
		value = pool.create<uint32>();
		PR_CHECK_NOT_EQ(value, NULL);
	}
	PR_CHECK_FALSE(pool.isEmpty());
	PR_CHECK_TRUE(pool.isFull());
	PR_CHECK_THROW(pool.create<uint32>());
}
PR_END_TESTCASE()

PR_BEGIN_TESTCASE(MemoryStack)
PR_TEST("Allocate")
{
	MemoryStack stack(1024);

	PR_CHECK_EQ(stack.usedMemory(), 0);
	uint32* value = stack.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_EQ(stack.usedMemory(), sizeof(uint32));
	stack.freeAll();
	PR_CHECK_EQ(stack.usedMemory(), 0);
}
PR_TEST("Marker")
{
	MemoryStack stack(1024);

	PR_CHECK_EQ(stack.markerCount(), 0);
	PR_CHECK_EQ(stack.usedMemory(), 0);
	uint32* value = stack.create<uint32>();
	PR_CHECK_NOT_EQ(value, NULL);
	PR_CHECK_EQ(stack.markerCount(), 0);
	PR_CHECK_EQ(stack.usedMemory(), sizeof(uint32));
	stack.mark();
	PR_CHECK_EQ(stack.markerCount(), 1);
	uint32* value2 = stack.create<uint32>();
	PR_CHECK_NOT_EQ(value2, NULL);
	PR_CHECK_GREAT(stack.usedMemory(), sizeof(uint32));
	stack.freeUntilMarker();
	PR_CHECK_EQ(stack.markerCount(), 0);
	PR_CHECK_EQ(stack.usedMemory(), sizeof(uint32));
	stack.freeUntilMarker();
	PR_CHECK_EQ(stack.markerCount(), 0);
	PR_CHECK_EQ(stack.usedMemory(), 0);
}
PR_TEST("Full")
{
	MemoryStack stack(1024);

	PR_CHECK_TRUE(stack.isEmpty());
	PR_CHECK_FALSE(stack.isFull());
	void* ptr = stack.allocate(1024);
	PR_CHECK_NOT_EQ(ptr, NULL);
	PR_CHECK_FALSE(stack.isEmpty());
	PR_CHECK_TRUE(stack.isFull());
	stack.freeAll();
	PR_CHECK_TRUE(stack.isEmpty());
	PR_CHECK_FALSE(stack.isFull());
}
PR_TEST("Exception")
{
	MemoryStack stack(1024);

	PR_CHECK_TRUE(stack.isEmpty());
	PR_CHECK_FALSE(stack.isFull());
	void* ptr = stack.allocate(1020);
	PR_CHECK_NOT_EQ(ptr, NULL);
	PR_CHECK_FALSE(stack.isEmpty());
	PR_CHECK_FALSE(stack.isFull());
	PR_CHECK_THROW(stack.create<uint64>());
	PR_CHECK_FALSE(stack.isEmpty());
	PR_CHECK_FALSE(stack.isFull());
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(MemoryPool);
PRT_TESTCASE(MemoryStack);
PRT_END_MAIN