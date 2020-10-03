#include "FrameBufferContainer.h"

namespace PR {
FrameBufferContainer::FrameBufferContainer(const Size2i& size, Size1i specChannels)
	: mSpectral(new FrameBufferFloat(std::max<Size1i>(3, specChannels), size, 0.0f))
{
	mInt1D[AOV_PixelWeight]		 = std::make_shared<FrameBufferFloat>(1, size, 0);
	mIntCounter[AOV_SampleCount] = std::make_shared<FrameBufferUInt32>(1, size, 0);
	mIntCounter[AOV_Feedback]	 = std::make_shared<FrameBufferUInt32>(1, size, 0);
}

FrameBufferContainer::~FrameBufferContainer()
{
}

void FrameBufferContainer::clear(bool force)
{
	mSpectral->clear(force);

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

	for (auto& pair : mLPE_Spectral)
		pair.second->clear(force);

	for (auto p : mCustom1D)
		p.second->clear(force);

	for (auto p : mCustomCounter)
		p.second->clear(force);

	for (auto p : mCustom3D)
		p.second->clear(force);

	for (auto p : mCustomSpectral)
		p.second->clear(force);
}

std::shared_ptr<FrameBufferFloat> FrameBufferContainer::createSpectralBuffer() const
{
	return std::make_shared<FrameBufferFloat>(mSpectral->channels(), mSpectral->size(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> FrameBufferContainer::create3DBuffer() const
{
	return std::make_shared<FrameBufferFloat>(3, mSpectral->size(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> FrameBufferContainer::create1DBuffer() const
{
	return std::make_shared<FrameBufferFloat>(1, mSpectral->size(), 0.0f);
}

std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::createCounterBuffer() const
{
	return std::make_shared<FrameBufferUInt32>(1, mSpectral->size(), 0);
}

} // namespace PR
