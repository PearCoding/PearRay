#include "BufferSerializer.h"

namespace PR {
BufferSerializer::BufferSerializer()
	: Serializer(false)
	, mSource(nullptr)
	, mBuffer()
	, mIt(0)
	, mBufferIt(0)
{
}

BufferSerializer::BufferSerializer(Serializer* source, size_t bufferSize)
	: Serializer(source->isReadMode())
	, mSource(source)
	, mBuffer(bufferSize)
	, mIt(0)
	, mBufferIt(0)
{
}

BufferSerializer::~BufferSerializer()
{
}

void BufferSerializer::reset(Serializer* source, size_t bufferSize)
{
	mSource = source;
	mBuffer.resize(bufferSize);
	mIt		  = 0;
	mBufferIt = 0;
	setReadMode(source->isReadMode());
}

bool BufferSerializer::isValid() const
{
	return mSource != nullptr;
}

void BufferSerializer::flush()
{
	if (!isReadMode()) {
		if (mIt == 0)
			return;

		PR_ASSERT(mIt == mBufferIt, "In write mode buffer it and it do not differ");
		mSource->writeRaw(mBuffer.data(), mIt);
		mIt		  = 0;
		mBufferIt = 0;
	} else if (mIt >= mBuffer.size()) { // End of buffer
		mIt		  = 0;
		mBufferIt = 0;
	}
}

void BufferSerializer::fetch()
{
	if (!isReadMode() || mBufferIt == 0)
		return;

	flush();

	size_t remaining = mBuffer.size() - mBufferIt;
	PR_ASSERT(remaining > 0, "After flush there should be some space!");
	mBufferIt += mSource->readRaw(mBuffer.data() + mBufferIt, remaining);
}

size_t BufferSerializer::writeRaw(const uint8* data, size_t size)
{
	PR_ASSERT(isValid(), "Trying to write into a close buffer!");
	PR_ASSERT(!isReadMode(), "Trying to write into a read serializer!");

	size_t written = 0;
	while (written < size) {
		size_t remainingBuffer = mBuffer.size() - mIt;
		size_t remainingSrc	   = size - written;
		size_t writesize	   = std::min(remainingBuffer, remainingSrc);

		std::memcpy(mBuffer.data() + mIt, data + written, writesize);
		mIt += writesize;

		if (mIt >= mBuffer.size())
			flush();
	}

	mBufferIt = mIt;
	return size;
}

size_t BufferSerializer::readRaw(uint8* data, size_t size)
{
	PR_ASSERT(isValid(), "Trying to read from a close buffer!");
	PR_ASSERT(isReadMode(), "Trying to read from a write serializer!");
	PR_ASSERT(mIt <= mBufferIt, "Read iterator has to be lower than buffer iterator");

	size_t read = 0;
	while (read < size) {
		size_t remainingBuffer = mBufferIt - mIt;
		size_t remainingSrc	   = size - read;
		size_t readsize		   = std::min(remainingBuffer, remainingSrc);

		std::memcpy(data + read, mBuffer.data() + mIt, readsize);
		mIt += readsize;

		if (mIt >= mBufferIt)
			fetch();
	}

	return size;
}

} // namespace PR