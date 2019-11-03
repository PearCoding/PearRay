#include "HitStream.h"

//FIXME: RadixSort has a bug which chrashes the software!
//#define PR_USE_RADIXSORT

#ifdef PR_USE_RADIXSORT
#include "container/RadixSort.h"
#else
#include "container/QuickSort.h"
#endif

namespace PR {

HitStream::HitStream(size_t size)
	: mSize(size + size % PR_SIMD_BANDWIDTH)
	, mCurrentPos(0)
{
	mRayID.reserve(mSize);
	mSessionRayID.reserve(mSize);
	mMaterialID.reserve(mSize);
	mEntityID.reserve(mSize);
	mPrimitiveID.reserve(mSize);
	for (int i = 0; i < 2; ++i) {
		mUV[i].reserve(mSize);
	}
	mFlags.reserve(mSize);
}

HitStream::~HitStream()
{
}

void HitStream::add(const HitEntry& entry)
{
	PR_ASSERT(!isFull(), "Check before adding!");

	mRayID.emplace_back(entry.RayID);
	mSessionRayID.emplace_back(entry.SessionRayID);
	mMaterialID.emplace_back(entry.MaterialID);
	mEntityID.emplace_back(entry.EntityID);
	mPrimitiveID.emplace_back(entry.PrimitiveID);
	mUV[0].emplace_back(entry.UV[0]);
	mUV[1].emplace_back(entry.UV[1]);
	mFlags.emplace_back(entry.Flags);
}

HitEntry HitStream::get(size_t index) const
{
	HitEntry entry;
	entry.Flags		   = mFlags[index];
	entry.RayID		   = mRayID[index];
	entry.SessionRayID = mSessionRayID[index];
	entry.MaterialID   = mMaterialID[index];
	entry.EntityID	 = mEntityID[index];
	entry.PrimitiveID  = mPrimitiveID[index];
	entry.UV[0]		   = mUV[0][index];
	entry.UV[1]		   = mUV[1][index];

	return entry;
}

void HitStream::sort()
{
	if (currentSize() == 0)
		return;

	// Swapping the whole context is quite heavy,
	// but a single vector solution requires additional memory
	auto op = [&](size_t a, size_t b) {
		std::swap(mRayID[a], mRayID[b]);
		std::swap(mSessionRayID[a], mSessionRayID[b]);
		std::swap(mMaterialID[a], mMaterialID[b]);
		std::swap(mEntityID[a], mEntityID[b]);
		std::swap(mPrimitiveID[a], mPrimitiveID[b]);
		std::swap(mUV[0][a], mUV[0][b]);
		std::swap(mUV[1][a], mUV[1][b]);
		std::swap(mFlags[a], mFlags[b]);
	};

#ifdef PR_USE_RADIXSORT
	uint32 mask = vem(currentSize());
	radixSort(mEntityID.data(), op,
			  0, currentSize() - 1, mask);
#else
	quickSort(mEntityID.data(), op,
			  0, currentSize() - 1);
#endif

	size_t end = currentSize();
	for (size_t i = 0; i < end;) {
		size_t start  = i;
		uint32 entity = mEntityID[i];

		while (i < end && mEntityID[i] == entity) {
			++i;
		}
		size_t s = i - start;

		if (s > 2) {
#ifdef PR_USE_RADIXSORT
			radixSort(mMaterialID.data(), op,
					  start, i - 1, mask);
#else
			quickSort(mMaterialID.data(), op,
					  start, i - 1);
#endif
		}
	}

	mCurrentPos = 0;
}

void HitStream::reset()
{
	mRayID.clear();
	mSessionRayID.clear();
	mMaterialID.clear();
	mEntityID.clear();
	mPrimitiveID.clear();
	mUV[0].clear();
	mUV[1].clear();
	mFlags.clear();

	mCurrentPos = 0;
}

ShadingGroup HitStream::getNextGroup()
{
	PR_ASSERT(hasNextGroup(), "Never call when not available");
	ShadingGroup grp;
	grp.Stream	 = this;
	grp.EntityID   = mEntityID[mCurrentPos];
	grp.MaterialID = mMaterialID[mCurrentPos];
	grp.Start	  = mCurrentPos;

	while (mCurrentPos < currentSize()
		   && mEntityID[mCurrentPos] == grp.EntityID
		   && mMaterialID[mCurrentPos] == grp.MaterialID) {
		++mCurrentPos;
	}

	grp.End = mCurrentPos - 1;

	return grp;
}

size_t HitStream::getMemoryUsage() const
{
	return mSize * (5 * sizeof(uint32) + 2 * sizeof(float) + sizeof(uint8));
}

} // namespace PR
