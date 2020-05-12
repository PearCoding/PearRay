#include "LightPathBuffer.h"

namespace PR {
struct RowHeader {
	size_t Entries;
};

LightPathBuffer::LightPathBuffer(size_t maxRays, size_t maxLength)
	: mMaxRays(maxRays)
	, mMaxLength(maxLength)
	, mRowStride(sizeof(RowHeader) + sizeof(LightPathBufferEntry) * maxLength)
	, mData(maxRays * mRowStride)
{
}

void LightPathBuffer::reset()
{
	std::fill(mData.begin(), mData.end(), 0);
}

bool LightPathBuffer::add(size_t ray_id, const LightPathBufferEntry& entry)
{
	if (ray_id >= mMaxRays)
		return false;

	RowHeader* header = reinterpret_cast<RowHeader*>(&mData[ray_id * mRowStride]);
	if (header->Entries >= mMaxLength)
		return false;

	LightPathBufferEntry* entries = reinterpret_cast<LightPathBufferEntry*>((uint8*)header + sizeof(RowHeader));
	entries[header->Entries]	  = entry;

	header->Entries += 1;

	return true;
}

size_t LightPathBuffer::length(size_t ray_id) const
{
	if (ray_id >= mMaxRays)
		return 0;

	const RowHeader* header = reinterpret_cast<const RowHeader*>(&mData[ray_id * mRowStride]);
	return header->Entries;
}

LightPath LightPathBuffer::getPath(size_t ray_id, size_t maxLength) const
{
	if (ray_id >= mMaxRays)
		return LightPath();

	const RowHeader* header				= reinterpret_cast<const RowHeader*>(&mData[ray_id * mRowStride]);
	const LightPathBufferEntry* entries = reinterpret_cast<const LightPathBufferEntry*>((uint8*)header + sizeof(RowHeader));
	const size_t len					= std::min(header->Entries, maxLength);

	LightPath path(1 + len);
	path.addToken(LightPathToken(ST_CAMERA, SE_NONE));

	for (size_t i = 0; i < len; ++i) {
		path.addToken(entries[i].tokenize());
	}

	return path;
}

LightPathBufferEntry LightPathBuffer::getEntry(size_t ray_id, size_t entry) const
{
	if (ray_id >= mMaxRays)
		return LightPathBufferEntry();

	const RowHeader* header				= reinterpret_cast<const RowHeader*>(&mData[ray_id * mRowStride]);
	const LightPathBufferEntry* entries = reinterpret_cast<const LightPathBufferEntry*>((uint8*)header + sizeof(RowHeader));

	return (entry < header->Entries)
			   ? entries[entry]
			   : LightPathBufferEntry();
}
} // namespace PR