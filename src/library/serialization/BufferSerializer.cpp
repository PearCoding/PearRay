#include "BufferSerializer.h"

namespace PR {
BufferSerializer::BufferSerializer(uint32 version)
	: Serializer(false, version)
	, mBuffer(nullptr)
	, mIt(0)
{
}

BufferSerializer::BufferSerializer(std::vector<uint8>* buffer, bool readmode, uint32 version)
	: Serializer(readmode, version)
	, mBuffer(buffer)
	, mIt(0)
{
	open(buffer, readmode);
}

BufferSerializer::~BufferSerializer()
{
	close();
}

bool BufferSerializer::open(std::vector<uint8>* buffer, bool readmode)
{
	mBuffer = buffer;
	mIt		= 0;
	if (!mBuffer)
		return false;

	setReadMode(readmode);
	if (isReadMode()) {
		uint32 v = version();
		read(v);
		setVersion(v);
	} else {
		write(version());
	}

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
	mBuffer->reserve(mBuffer->size() + size);
	for (size_t i = 0; i < size; ++i)
		mBuffer->emplace_back(data[i]);
}

void BufferSerializer::readRaw(uint8* data, size_t elems, size_t elemSize)
{
	PR_ASSERT(isValid(), "Trying to read from a close buffer!");
	PR_ASSERT(isReadMode(), "Trying to read from a write serializer!");

	const size_t size = elems * elemSize;
	if (mBuffer->size() - mIt < size)
		return;

	for (size_t i = 0; i < size; ++i, ++mIt)
		data[i] = mBuffer->at(mIt);
}

} // namespace PR