#include "OutputBufferData.h"

namespace PR {
OutputBufferData::OutputBufferData(size_t width, size_t height, size_t specChannels)
	: mSpectral(new FrameBufferFloat(std::max<size_t>(3, specChannels), width, height, 0.0f))
{
	mIntCounter[AOV_SampleCount] = std::make_shared<FrameBufferUInt32>(1, width, height, 0);
	mIntCounter[AOV_Feedback]	= std::make_shared<FrameBufferUInt32>(1, width, height, 0);
}

OutputBufferData::~OutputBufferData()
{
}

void OutputBufferData::clear(bool force)
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

std::shared_ptr<FrameBufferFloat> OutputBufferData::createSpectralBuffer() const
{
	return std::make_shared<FrameBufferFloat>(mSpectral->channels(), mSpectral->width(), mSpectral->height(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> OutputBufferData::create3DBuffer() const
{
	return std::make_shared<FrameBufferFloat>(3, mSpectral->width(), mSpectral->height(), 0.0f);
}

std::shared_ptr<FrameBufferFloat> OutputBufferData::create1DBuffer() const
{
	return std::make_shared<FrameBufferFloat>(1, mSpectral->width(), mSpectral->height(), 0.0f);
}

std::shared_ptr<FrameBufferUInt32> OutputBufferData::createCounterBuffer() const
{
	return std::make_shared<FrameBufferUInt32>(1, mSpectral->width(), mSpectral->height(), 0);
}

} // namespace PR
