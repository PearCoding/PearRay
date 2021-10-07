#include "FrameOutputDevice.h"
#include "LocalFrameOutputDevice.h"
#include "filter/IFilter.h"

namespace PR {
FrameOutputDevice::FrameOutputDevice(const std::shared_ptr<IFilter>& filter,
									 const Size2i& size, Size1i specChannels, bool monotonic)
	: OutputDevice()
	, mFilter(filter)
	, mMonotonic(monotonic)
	, mData(size, specChannels)
{
}

FrameOutputDevice::~FrameOutputDevice()
{
}

// Do not propagate the variance aovs to local, as it is handled in global entirely
inline static bool ignoreInLocal(AOVSpectral var)
{
	return var == AOV_OnlineMean || var == AOV_OnlineVariance;
}

std::shared_ptr<LocalOutputDevice> FrameOutputDevice::createLocal(const Size2i& size) const
{
	std::shared_ptr<LocalFrameOutputDevice> bucket = std::make_shared<LocalFrameOutputDevice>(
		mFilter, size,
		mData.getInternalChannel_Spectral(AOV_Output)->channels(), mMonotonic);

	// Internals
	for (int i = 0; i < AOV_SPECTRAL_COUNT; ++i) {
		if (!ignoreInLocal((AOVSpectral)i))
			bucket->data().requestInternalChannel_Spectral((AOVSpectral)i);
	}
	for (int i = 0; i < AOV_3D_COUNT; ++i)
		bucket->data().requestInternalChannel_3D((AOV3D)i);
	for (int i = 0; i < AOV_1D_COUNT; ++i)
		bucket->data().requestInternalChannel_1D((AOV1D)i);
	for (int i = 0; i < AOV_COUNTER_COUNT; ++i)
		bucket->data().requestInternalChannel_Counter((AOVCounter)i);

	// Custom (TODO: This can be done more efficiently)
	for (uint32 id = 0; id < mData.mCustom3D.size(); ++id)
		bucket->data().requestCustomChannel_3D(id);
	for (uint32 id = 0; id < mData.mCustom1D.size(); ++id)
		bucket->data().requestCustomChannel_1D(id);
	for (uint32 id = 0; id < mData.mCustomCounter.size(); ++id)
		bucket->data().requestCustomChannel_Counter(id);
	for (uint32 id = 0; id < mData.mCustomSpectral.size(); ++id)
		bucket->data().requestCustomChannel_Spectral(id);

	// LPE (TODO: This can be done more efficiently)
	for (int i = 0; i < AOV_3D_COUNT; ++i) {
		uint32 id = 0;
		for (const auto& p : mData.mLPE_3D[i])
			bucket->data().requestLPEChannel_3D((AOV3D)i, p.first, id++);
	}
	for (int i = 0; i < AOV_1D_COUNT; ++i) {
		uint32 id = 0;
		for (const auto& p : mData.mLPE_1D[i])
			bucket->data().requestLPEChannel_1D((AOV1D)i, p.first, id++);
	}
	for (int i = 0; i < AOV_COUNTER_COUNT; ++i) {
		uint32 id = 0;
		for (const auto& p : mData.mLPE_Counter[i])
			bucket->data().requestLPEChannel_Counter((AOVCounter)i, p.first, id++);
	}
	for (int i = 0; i < AOV_SPECTRAL_COUNT; ++i) {
		if (ignoreInLocal((AOVSpectral)i))
			continue;

		uint32 id = 0;
		for (const auto& p : mData.mLPE_Spectral[i])
			bucket->data().requestLPEChannel_Spectral((AOVSpectral)i, p.first, id++);
	}

	bucket->cache();

	return bucket;
}

void FrameOutputDevice::mergeLocal(
	const Point2i& p,
	const std::shared_ptr<LocalOutputDevice>& local,
	size_t iteration)
{
	const auto bucket = std::dynamic_pointer_cast<LocalFrameOutputDevice>(local);
	if (!bucket)
		return;

	std::lock_guard<std::mutex> guard(mMergeMutex);

	const int32 off		 = mFilter->radius();
	const Size2i offSize = Size2i(off, off);
	const Point2i sp	 = p - offSize;

	const Point2i dst_off = Point2i::Zero().cwiseMax(sp);
	const Point2i src_off = -Point2i::Zero().cwiseMin(sp);

	const Size2i src_size = Size2i::fromArray(bucket->extendedSize() - src_off);
	const Size2i dst_size = src_size;

	// Do the variance estimation
	if (mData.hasVarianceEstimator()) {
		auto varianceEstimator = mData.varianceEstimator();
		for (Size1i i = 0; i < mData.mSpectral[AOV_OnlineMean]->channels(); ++i)
			varianceEstimator.addBlock(i, dst_off, dst_size, src_off, src_size, *bucket->data().mSpectral[AOV_Output], iteration);
	}

	// Add spectral AOVs
	PR_OPT_LOOP
	for (int i = 0; i < AOV_SPECTRAL_COUNT; ++i) {
		if (ignoreInLocal((AOVSpectral)i))
			continue;

		if (mData.mSpectral[i])
			mCopySpectral[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mSpectral[i]);

		PR_OPT_LOOP
		for (size_t k = 0; k < mData.mLPE_Spectral[i].size(); ++k)
			mCopyLPE_Spectral[i][k]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Spectral[i][k].second);
	}

	// Add 3d AOVs
	PR_OPT_LOOP
	for (int i = 0; i < AOV_3D_COUNT; ++i) {
		if (mData.mInt3D[i])
			mData.mInt3D[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mInt3D[i]);

		PR_OPT_LOOP
		for (size_t k = 0; k < mData.mLPE_3D[i].size(); ++k)
			mData.mLPE_3D[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_3D[i][k].second);
	}

	// Add 1d AOVs
	PR_OPT_LOOP
	for (int i = 0; i < AOV_1D_COUNT; ++i) {
		if (mData.mInt1D[i])
			mData.mInt1D[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mInt1D[i]);

		PR_OPT_LOOP
		for (size_t k = 0; k < mData.mLPE_1D[i].size(); ++k)
			mData.mLPE_1D[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_1D[i][k].second);
	}

	// Add counter AOVs
	PR_OPT_LOOP
	for (int i = 0; i < AOV_COUNTER_COUNT; ++i) {
		if (i == AOV_Feedback) {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->applyBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mIntCounter[i],
												 [](uint32 pre, uint32 val) { return pre | val; });

			PR_OPT_LOOP
			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->applyBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Counter[i][k].second,
															[](uint32 pre, uint32 val) { return pre | val; });
		} else {
			if (mData.mIntCounter[i])
				mData.mIntCounter[i]->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mIntCounter[i]);

			PR_OPT_LOOP
			for (size_t k = 0; k < mData.mLPE_Counter[i].size(); ++k)
				mData.mLPE_Counter[i][k].second->addBlock(dst_off, dst_size, src_off, src_size, *bucket->data().mLPE_Counter[i][k].second);
		}
	}

	// Add custom spectral aovs
	PR_OPT_LOOP
	for (auto aI = mData.mCustomSpectral.begin(), bI = bucket->data().mCustomSpectral.begin();
		 aI != mData.mCustomSpectral.end();
		 ++aI, ++bI) {
		(*aI)->addBlock(dst_off, dst_size, src_off, src_size, *(*bI));
	}

	// Add custom 3d aovs
	PR_OPT_LOOP
	for (auto aI = mData.mCustom3D.begin(), bI = bucket->data().mCustom3D.begin();
		 aI != mData.mCustom3D.end();
		 ++aI, ++bI) {
		(*aI)->addBlock(dst_off, dst_size, src_off, src_size, *(*bI));
	}

	// Add custom 1d aovs
	PR_OPT_LOOP
	for (auto aI = mData.mCustom1D.begin(), bI = bucket->data().mCustom1D.begin();
		 aI != mData.mCustom1D.end();
		 ++aI, ++bI) {
		(*aI)->addBlock(dst_off, dst_size, src_off, src_size, *(*bI));
	}

	// Add custom counter aovs
	PR_OPT_LOOP
	for (auto aI = mData.mCustomCounter.begin(), bI = bucket->data().mCustomCounter.begin();
		 aI != mData.mCustomCounter.end();
		 ++aI, ++bI) {
		(*aI)->addBlock(dst_off, dst_size, src_off, src_size, *(*bI));
	}
}

void FrameOutputDevice::onEndOfIteration(size_t iteration)
{
	const auto merger = [=](float a, float b) { return (a * (iteration - 1) + b) / iteration; };
	PR_OPT_LOOP
	for (int i = 0; i < AOV_SPECTRAL_COUNT; ++i) {
		if (ignoreInLocal((AOVSpectral)i))
			continue;

		if (mData.mSpectral[i]) {
			mData.mSpectral[i]->applyBlock(Point2i::Zero(), *mCopySpectral[i], merger);
			mCopySpectral[i]->clear(true);
		}

		PR_OPT_LOOP
		for (size_t k = 0; k < mData.mLPE_Spectral[i].size(); ++k) {
			mData.mLPE_Spectral[i][k].second->applyBlock(Point2i::Zero(), *mCopyLPE_Spectral[i][k], merger);
			mCopyLPE_Spectral[i][k]->clear(true);
		}
	}
}

void FrameOutputDevice::clear(bool force)
{
	mData.clear(force);
}

void FrameOutputDevice::enable1DChannel(AOV1D var)
{
	mData.requestInternalChannel_1D(var);
}

void FrameOutputDevice::enableCounterChannel(AOVCounter var)
{
	mData.requestInternalChannel_Counter(var);
}

void FrameOutputDevice::enable3DChannel(AOV3D var)
{
	mData.requestInternalChannel_3D(var);
}

void FrameOutputDevice::enableSpectralChannel(AOVSpectral var)
{
	mData.requestInternalChannel_Spectral(var);
	if (!mCopySpectral[var])
		mCopySpectral[var] = mData.createSpectralBuffer();
	mCopySpectral[var]->clear(true);
}

void FrameOutputDevice::registerLPE1DChannel(AOV1D var, const LightPathExpression& expr, uint32 id)
{
	mData.requestLPEChannel_1D(var, expr, id);
}

void FrameOutputDevice::registerLPECounterChannel(AOVCounter var, const LightPathExpression& expr, uint32 id)
{
	mData.requestLPEChannel_Counter(var, expr, id);
}

void FrameOutputDevice::registerLPE3DChannel(AOV3D var, const LightPathExpression& expr, uint32 id)
{
	mData.requestLPEChannel_3D(var, expr, id);
}

void FrameOutputDevice::registerLPESpectralChannel(AOVSpectral var, const LightPathExpression& expr, uint32 id)
{
	mData.requestLPEChannel_Spectral(var, expr, id);
	if (id >= mCopyLPE_Spectral[var].size())
		mCopyLPE_Spectral[var].resize(id);
	mCopyLPE_Spectral[var][id] = mData.createSpectralBuffer();
	mCopyLPE_Spectral[var][id]->clear(true);
}

void FrameOutputDevice::registerCustom1DChannel(const std::string&, uint32 id)
{
	mData.requestCustomChannel_1D(id);
}

void FrameOutputDevice::registerCustomCounterChannel(const std::string&, uint32 id)
{
	mData.requestCustomChannel_Counter(id);
}

void FrameOutputDevice::registerCustom3DChannel(const std::string&, uint32 id)
{
	mData.requestCustomChannel_3D(id);
}

void FrameOutputDevice::registerCustomSpectralChannel(const std::string&, uint32 id)
{
	mData.requestCustomChannel_Spectral(id);
}

} // namespace PR
