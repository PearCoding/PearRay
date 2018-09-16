#include "HitStream.h"
#include "container/RadixSort.h"
#include "math/SIMD.h"

namespace PR {

HitStream::HitStream(size_t size)
	: mSize(size + size % PR_SIMD_BANDWIDTH)
	, mEnd(0)
	, mCurrentPos(0)
{
	mRayID.resize(mSize);
	mMaterialID.resize(mSize);
	mEntityID.resize(mSize);
	mPrimitiveID.resize(mSize);
	for (int i = 0; i < 2; ++i) {
		mUV[i].resize(mSize);
	}
	mFlags.resize(mSize);
}

HitStream::~HitStream()
{
}

void HitStream::sort()
{
	// Swapping the whole context is quite heavy,
	// but a single vector solution requires additional memory
	auto op = [&](size_t a, size_t b) {
		std::swap(mRayID[a], mRayID[b]);
		std::swap(mMaterialID[a], mMaterialID[b]);
		std::swap(mEntityID[a], mEntityID[b]);
		std::swap(mPrimitiveID[a], mPrimitiveID[b]);
		std::swap(mUV[0][a], mUV[0][b]);
		std::swap(mUV[1][a], mUV[1][b]);
		std::swap(mFlags[a], mFlags[b]);
	};

	// TODO: Maybe get maximum mask?
	radixSort(mEntityID.data(), op,
			  0, mEnd - 1);

	for (size_t i = 0; i < mEnd;) {
		size_t start  = i;
		uint32 entity = mEntityID[i];

		while (i < mEnd && mEntityID[i] == entity) {
			++i;
		}
		size_t s = i - start;

		if (s > 2) {
			radixSort(mMaterialID.data(), op,
					  start, i - 1, (uint32)(1 << 31));
		}
	}
}

void HitStream::reset()
{
	mEnd		= 0;
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

	while (mCurrentPos < mEnd
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
