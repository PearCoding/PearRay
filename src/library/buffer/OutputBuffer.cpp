#include "OutputBuffer.h"
#include "Feedback.h"
#include "OutputBufferBucket.h"
#include "filter/IFilter.h"
#include "shader/ShadingPoint.h"

namespace PR {
OutputBuffer::OutputBuffer(const std::shared_ptr<IFilter>& filter,
						   size_t width, size_t height, size_t specChannels)
	: mFilter(filter)
	, mData(width, height, specChannels)
{
}

OutputBuffer::~OutputBuffer()
{
}

std::shared_ptr<OutputBufferBucket> OutputBuffer::createBucket(size_t width, size_t height) const
{
	std::shared_ptr<OutputBufferBucket> bucket = std::make_shared<OutputBufferBucket>(
		mFilter, width, height,
		mData.getInternalChannel_Spectral()->channels());

	// Internals
	for (int i = 0; i < AOV_3D_COUNT; ++i)
		bucket->data().requestInternalChannel_3D((AOV3D)i);
	for (int i = 0; i < AOV_1D_COUNT; ++i)
		bucket->data().requestInternalChannel_1D((AOV1D)i);
	for (int i = 0; i < AOV_COUNTER_COUNT; ++i)
		bucket->data().requestInternalChannel_Counter((AOVCounter)i);

	// Custom
	for (const auto& p : mData.mCustom3D)
		bucket->data().requestCustomChannel_3D(p.first);
	for (const auto& p : mData.mCustom1D)
		bucket->data().requestCustomChannel_1D(p.first);
	for (const auto& p : mData.mCustomCounter)
		bucket->data().requestCustomChannel_Counter(p.first);
	for (const auto& p : mData.mCustomSpectral)
		bucket->data().requestCustomChannel_Spectral(p.first);

	// LPE
	size_t tmp;
	for (int i = 0; i < AOV_3D_COUNT; ++i)
		for (const auto& p : mData.mLPE_3D[i])
			bucket->data().requestLPEChannel_3D((AOV3D)i, p.first, tmp);
	for (int i = 0; i < AOV_1D_COUNT; ++i)
		for (const auto& p : mData.mLPE_1D[i])
			bucket->data().requestLPEChannel_1D((AOV1D)i, p.first, tmp);
	for (int i = 0; i < AOV_COUNTER_COUNT; ++i)
		for (const auto& p : mData.mLPE_Counter[i])
			bucket->data().requestLPEChannel_Counter((AOVCounter)i, p.first, tmp);
	for (const auto& p : mData.mLPE_Spectral)
		bucket->data().requestLPEChannel_Spectral(p.first, tmp);

	bucket->cache();

	return bucket;
}

void OutputBuffer::mergeBucket(size_t ox, size_t oy,
							   const std::shared_ptr<OutputBufferBucket>& bucket)
{
	std::lock_guard<std::mutex> guard(mMergeMutex);

	const int32 off = mFilter->radius();
	int32 px		= (int32)ox - off;
	int32 py		= (int32)oy - off;

	ox = std::max(0, px);
	oy = std::max(0, py);

	const size_t dx = -std::min(0, px);
	const size_t dy = -std::min(0, py);

	mData.mSpectral->addBlock(ox, oy, dx, dy, *bucket->data().mSpectral);
	for (size_t k = 0; k < mData.mLPE_Spectral.size(); ++k)
		mData.mLPE_Spectral[k].second->addBlock(ox, oy, dx, dy, *bucket->data().mLPE_Spectral[k].second);

	for (int i = 0; i < AOV_3D_COUNT; ++i) {
		if (mData.mInt3D[i])
			mData.mInt3D[i]->addBlock(ox, oy, dx, dy, *bucket->data().mInt3D[i]);

		for (size_t k = 0; k < mData.mLPE_3D[i].size(); ++k)
			mData.mLPE_3D[i][k].second->addBlock(ox, oy, dx, dy, *bucket->data().mLPE_3D[i][k].second);
	}

	for (int i = 0; i < AOV_1D_COUNT; ++i) {
		if (mData.mInt1D[i])
			mData.mInt1D[i]->addBlock(ox, oy, dx, dy, *bucket->data().mInt1D[i]);

		for (size_t k = 0; k < mData.mLPE_1D[i].size(); ++k)
			mData.mLPE_1D[i][k].second->addBlock(ox, oy, dx, dy, *bucket->data().mLPE_1D[i][k].second);
	}

	for (int i = 0; i < AOV_COUNTER_COUNT; ++i) {
		if (i == AOV_Feedback) {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->applyBlock(ox, oy, dx, dy, *bucket->data().mIntCounter[i],
												 [](uint32 pre, uint32 val) { return pre | val; });

			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->applyBlock(ox, oy, dx, dy, *bucket->data().mLPE_Counter[i][k].second,
															[](uint32 pre, uint32 val) { return pre | val; });
		} else {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->addBlock(ox, oy, dx, dy, *bucket->data().mIntCounter[i]);

			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->addBlock(ox, oy, dx, dy, *bucket->data().mLPE_Counter[i][k].second);
		}
	}

	// Custom
	for (auto aI = mData.mCustom3D.begin(), bI = bucket->data().mCustom3D.begin();
		 aI != mData.mCustom3D.end();
		 ++aI, ++bI) {
		aI->second->addBlock(ox, oy, dx, dy, *bI->second);
	}
	for (auto aI = mData.mCustom1D.begin(), bI = bucket->data().mCustom1D.begin();
		 aI != mData.mCustom1D.end();
		 ++aI, ++bI) {
		aI->second->addBlock(ox, oy, dx, dy, *bI->second);
	}
	for (auto aI = mData.mCustomCounter.begin(), bI = bucket->data().mCustomCounter.begin();
		 aI != mData.mCustomCounter.end();
		 ++aI, ++bI) {
		aI->second->addBlock(ox, oy, dx, dy, *bI->second);
	}
	for (auto aI = mData.mCustomSpectral.begin(), bI = bucket->data().mCustomSpectral.begin();
		 aI != mData.mCustomSpectral.end();
		 ++aI, ++bI) {
		aI->second->addBlock(ox, oy, dx, dy, *bI->second);
	}
}

} // namespace PR
