#include "BufferSerializer.h"

namespace PR {
BufferSerializer::BufferSerializer()
	: Serializer(false)
	, mBuffer(nullptr)
	, mSize(0)
	, mIt(0)
{
}

BufferSerializer::BufferSerializer(uint8* buffer, size_t size, bool readmode)
	: Serializer(readmode)
	, mBuffer(buffer)
	, mSize(size)
	, mIt(0)
{
	open(buffer, size, readmode);
}

BufferSerializer::~BufferSerializer()
{
	close();
}

bool BufferSerializer::open(uint8* buffer, size_t size, bool readmode)
{
	mBuffer = buffer;
	mSize	= size;
	mIt		= 0;
	if (!mBuffer)
		return false;

	setReadMode(readmode);

	return true;
}

void BufferSerializer::close()
{
	mBuffer = nullptr;
}

bool BufferSerializer::isValid() const
{
	return mBuffer;
}

void BufferSerializer::writeRaw(const uint8* data, size_t elems, size_t elemSize)
{
	PR_ASSERT(isValid(), "Trying to write into a close buffer!");
	PR_ASSERT(!isReadMode(), "Trying to write into a read serializer!");

	const size_t size = elems * elemSize;
	if (mIt + size > mSize)
		return; // TODO: Error?

	for (size_t i = 0; i < size; ++i)
		mBuffer[mIt + i] = data[i];

	mIt += size;
}

void BufferSerializer::readRaw(uint8* data, size_t elems, size_t elemSize)
{
	PR_ASSERT(isValid(), "Trying to read from a close buffer!");
	PR_ASSERT(isReadMode(), "Trying to read from a write serializer!");

	const size_t size = elems * elemSize;
	const size_t end  = std::min(mSize, mIt + size);

	for (size_t i = 0; mIt < end; ++i, ++mIt)
		data[i] = mBuffer[mIt];
}

} // namespace PR