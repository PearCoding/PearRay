#include "FrameOutputDevice.h"
#include "Feedback.h"
#include "FrameBufferBucket.h"
#include "filter/IFilter.h"
#include "trace/IntersectionPoint.h"

namespace PR {
FrameOutputDevice::FrameOutputDevice(Size1i specChannels)
	: mSpectralChannels(specChannels)
{
}

FrameOutputDevice::~FrameOutputDevice()
{
}

void FrameOutputDevice::commitSpectrals(const OutputCommitInformation& info, const OutputSpectralEntry* entries, size_t entrycount)
{
}

void FrameOutputDevice::commitShadingPoints(const OutputCommitInformation& info, const OutputShadingPointEntry* entries, size_t entrycount)
{
}

void FrameOutputDevice::commitFeedbacks(const OutputCommitInformation& info, const OutputFeedbackEntry* entries, size_t entrycount)
{
}

void FrameOutputDevice::onStart(RenderContext* ctx);
void FrameOutputDevice::onNextIteration();
void FrameOutputDevice::onStop();

std::shared_ptr<FrameBufferBucket> FrameOutputDevice::createBucket(const Size2i& size) const
{
	std::shared_ptr<FrameBufferBucket> bucket = std::make_shared<FrameBufferBucket>(
		mFilter, size,
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

void FrameOutputDevice::mergeBucket(const Point2i& p,
									const std::shared_ptr<FrameBufferBucket>& bucket)
{
	std::lock_guard<std::mutex> guard(mMergeMutex);

	const int32 off		 = mFilter->radius();
	const Size2i offSize = Size2i(off, off);
	const Point2i sp	 = p - offSize;

	const Point2i dst_off = Point2i::Zero().cwiseMax(sp);
	const Point2i src_off = -Point2i::Zero().cwiseMin(sp);

	const Size2i src_size = Size2i::fromArray(bucket->extendedViewSize() - src_off);
	const Size2i dst_size = src_size;

	mData.mSpectral->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mSpectral);
	for (size_t k = 0; k < mData.mLPE_Spectral.size(); ++k)
		mData.mLPE_Spectral[k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Spectral[k].second);

	for (int i = 0; i < AOV_3D_COUNT; ++i) {
		if (mData.mInt3D[i])
			mData.mInt3D[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mInt3D[i]);

		for (size_t k = 0; k < mData.mLPE_3D[i].size(); ++k)
			mData.mLPE_3D[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_3D[i][k].second);
	}

	for (int i = 0; i < AOV_1D_COUNT; ++i) {
		if (mData.mInt1D[i])
			mData.mInt1D[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mInt1D[i]);

		for (size_t k = 0; k < mData.mLPE_1D[i].size(); ++k)
			mData.mLPE_1D[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_1D[i][k].second);
	}

	for (int i = 0; i < AOV_COUNTER_COUNT; ++i) {
		if (i == AOV_Feedback) {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->applyBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mIntCounter[i],
												 [](uint32 pre, uint32 val) { return pre | val; });

			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->applyBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Counter[i][k].second,
															[](uint32 pre, uint32 val) { return pre | val; });
		} else {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mIntCounter[i]);

			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Counter[i][k].second);
		}
	}

	// Custom
	for (auto aI = mData.mCustom3D.begin(), bI = bucket->data().mCustom3D.begin();
		 aI != mData.mCustom3D.end();
		 ++aI, ++bI) {
		aI->second->addBlock(dst_off, dst_size, src_off, src_size, *bI->second);
	}
	for (auto aI = mData.mCustom1D.begin(), bI = bucket->data().mCustom1D.begin();
		 aI != mData.mCustom1D.end();
		 ++aI, ++bI) {
		aI->second->addBlock(dst_off, dst_size, src_off, src_size, *bI->second);
	}
	for (auto aI = mData.mCustomCounter.begin(), bI = bucket->data().mCustomCounter.begin();
		 aI != mData.mCustomCounter.end();
		 ++aI, ++bI) {
		aI->second->addBlock(dst_off, dst_size, src_off, src_size, *bI->second);
	}
	for (auto aI = mData.mCustomSpectral.begin(), bI = bucket->data().mCustomSpectral.begin();
		 aI != mData.mCustomSpectral.end();
		 ++aI, ++bI) {
		aI->second->addBlock(dst_off, dst_size, src_off, src_size, *bI->second);
	}
}

} // namespace PR
