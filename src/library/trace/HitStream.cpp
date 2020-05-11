#include "HitStream.h"
#include "Profiler.h"

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
	mMaterialID.reserve(mSize);
	mEntityID.reserve(mSize);
	mPrimitiveID.reserve(mSize);
	for (int i = 0; i < 3; ++i)
		mParameter[i].reserve(mSize);
	mFlags.reserve(mSize);
}

HitStream::~HitStream()
{
}

void HitStream::add(const HitEntry& entry)
{
	PR_PROFILE_THIS;

	PR_ASSERT(!isFull(), "Check before adding!");

	mRayID.emplace_back(entry.RayID);
	mMaterialID.emplace_back(entry.MaterialID);
	mEntityID.emplace_back(entry.EntityID);
	mPrimitiveID.emplace_back(entry.PrimitiveID);
	for (int i = 0; i < 3; ++i)
		mParameter[i].emplace_back(entry.Parameter[i]);
	mFlags.emplace_back(entry.Flags);
}

HitEntry HitStream::get(size_t index) const
{
	PR_PROFILE_THIS;

	HitEntry entry;
	entry.Flags		  = mFlags[index];
	entry.RayID		  = mRayID[index];
	entry.MaterialID  = mMaterialID[index];
	entry.EntityID	  = mEntityID[index];
	entry.PrimitiveID = mPrimitiveID[index];
	for (int i = 0; i < 3; ++i)
		entry.Parameter[i] = mParameter[i][index];

	return entry;
}

void HitStream::setup(bool sort)
{
	PR_PROFILE_THIS;

	if (currentSize() == 0)
		return;

	if (sort) {
		// Swapping the whole context is quite heavy,
		// but a single vector solution requires additional memory
		auto op = [&](size_t a, size_t b) {
			std::swap(mRayID[a], mRayID[b]);
			std::swap(mMaterialID[a], mMaterialID[b]);
			std::swap(mEntityID[a], mEntityID[b]);
			std::swap(mPrimitiveID[a], mPrimitiveID[b]);
			for (int i = 0; i < 3; ++i)
				std::swap(mParameter[i][a], mParameter[i][b]);
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
	}

	mCurrentPos = 0;
}

void HitStream::reset()
{
	PR_PROFILE_THIS;

	mRayID.clear();
	mMaterialID.clear();
	mEntityID.clear();
	mPrimitiveID.clear();
	for (int i = 0; i < 3; ++i)
		mParameter[i].clear();
	mFlags.clear();

	mCurrentPos = 0;
}

ShadingGroupBlock HitStream::getNextGroup()
{
	PR_PROFILE_THIS;

	PR_ASSERT(hasNextGroup(), "Never call when not available");
	ShadingGroupBlock grp;
	grp.Stream	   = this;
	grp.EntityID   = mEntityID[mCurrentPos];
	grp.MaterialID = mMaterialID[mCurrentPos];
	grp.Start	   = mCurrentPos;

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
	return mSize * (4 * sizeof(uint32) + 2 * sizeof(float) + sizeof(uint8));
}

} // namespace PR
