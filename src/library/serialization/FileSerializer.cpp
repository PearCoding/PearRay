#include "FileSerializer.h"
#include "Platform.h"

#include <fstream>

namespace PR {
struct FileSerializerInternal {
	std::fstream File;
	size_t MemoryFootprint = 0;
};

FileSerializer::FileSerializer(uint32 version)
	: Serializer(false, version)
	, mInternal(std::make_unique<FileSerializerInternal>())
{
}

FileSerializer::FileSerializer(const std::wstring& path, bool readmode, uint32 version)
	: Serializer(readmode, version)
	, mInternal(std::make_unique<FileSerializerInternal>())
{
	open(path, readmode);
}

FileSerializer::~FileSerializer()
{
	close();
}

size_t FileSerializer::memoryFootprint() const
{
	return mInternal->MemoryFootprint;
}

bool FileSerializer::open(const std::wstring& path, bool readmode)
{
	if (mInternal->File.is_open())
		return false;

	auto flags = (readmode ? std::ios_base::in : std::ios_base::out) | std::ios_base::binary;
	mInternal->File.open(encodePath(path), flags);
	if (!mInternal->File)
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

void FileSerializer::close()
{
	if (mInternal->File.is_open())
		mInternal->File.close();
}

bool FileSerializer::isValid() const
{
	return mInternal && mInternal->File.is_open();
}

void FileSerializer::writeRaw(const uint8* data, size_t elems, size_t elemSize)
{
	PR_ASSERT(isValid(), "Trying to write into a close buffer!");
	PR_ASSERT(!isReadMode(), "Trying to write into a read serializer!");

	const size_t size = elems * elemSize;
	mInternal->MemoryFootprint += size;
	mInternal->File.write(reinterpret_cast<const char*>(data), size);
}

void FileSerializer::readRaw(uint8* data, size_t elems, size_t elemSize)
{
	PR_ASSERT(isValid(), "Trying to read from a close buffer!");
	PR_ASSERT(isReadMode(), "Trying to read from a write serializer!");

	const size_t size = elems * elemSize;
	mInternal->MemoryFootprint += size;
	mInternal->File.read(reinterpret_cast<char*>(data), size);
}

} // namespace PR