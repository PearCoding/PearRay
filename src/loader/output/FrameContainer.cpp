#include "FrameContainer.h"

namespace PR {
FrameContainer::FrameContainer(const Size2i& size, Size1i specChannels)
{
	mSpectral[AOV_Output] = std::make_shared<FrameBufferFloat>(std::max<Size1i>(3, specChannels), size, 0.0f);
	PR_ASSERT(mSpectral[AOV_Output], "Spectral Output has to be available all the time");

	mIntCounter[AOV_SampleCount] = createCounterBuffer();
	mIntCounter[AOV_Feedback]	 = createCounterBuffer();
}

FrameContainer::~FrameContainer()
{
}

void FrameContainer::clear(bool force)
{
	if (mOnlineM) {
		mOnlineM->clear(force);
		mOnlineS->clear(force);
	}

	for (uint32 i = 0; i < AOV_SPECTRAL_COUNT; ++i) {
		if (mSpectral[i])
			mSpectral[i]->clear(force);
		for (auto& pair : mLPE_Spectral[i])
			pair.second->clear(force);
	}

	for (uint32 i = 0; i < AOV_1D_COUNT; ++i) {
		if (mInt1D[i])
			mInt1D[i]->clear(force);
		for (auto& pair : mLPE_1D[i])
			pair.second->clear(force);
	}

	for (uint32 i = 0; i < AOV_COUNTER_COUNT; ++i) {
		if (mIntCounter[i])
			mIntCounter[i]->clear(force);
		for (auto& pair : mLPE_Counter[i])
			pair.second->clear(force);
	}

	for (uint32 i = 0; i < AOV_3D_COUNT; ++i) {
		if (mInt3D[i])
			mInt3D[i]->clear(force);
		for (auto& pair : mLPE_3D[i])
			pair.second->clear(force);
	}

	for (auto p : mCustom1D)
		p->clear(force);

	for (auto p : mCustomCounter)
		p->clear(force);

	for (auto p : mCustom3D)
		p->clear(force);

	for (auto p : mCustomSpectral)
		p->clear(force);
}

std::shared_ptr<FrameBufferFloat> FrameContainer::createSpectralBuffer() const
{
	PR_ASSERT(mSpectral[AOV_Output], "Spectral Output has to be available all the time");
	return std::make_shared<FrameBufferFloat>(mSpectral[AOV_Output]->channels(), mSpectral[AOV_Output]->size(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> FrameContainer::create3DBuffer() const
{
	PR_ASSERT(mSpectral[AOV_Output], "Spectral Output has to be available all the time");
	return std::make_shared<FrameBufferFloat>(3, mSpectral[AOV_Output]->size(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> FrameContainer::create1DBuffer() const
{
	PR_ASSERT(mSpectral[AOV_Output], "Spectral Output has to be available all the time");
	return std::make_shared<FrameBufferFloat>(1, mSpectral[AOV_Output]->size(), 0.0f);
}

std::shared_ptr<FrameBufferUInt32> FrameContainer::createCounterBuffer() const
{
	PR_ASSERT(mSpectral[AOV_Output], "Spectral Output has to be available all the time");
	return std::make_shared<FrameBufferUInt32>(1, mSpectral[AOV_Output]->size(), 0);
}

} // namespace PR
