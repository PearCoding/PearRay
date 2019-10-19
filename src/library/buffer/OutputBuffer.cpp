#include "OutputBuffer.h"
#include "Feedback.h"
#include "renderer/RenderContext.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
OutputBuffer::OutputBuffer(RenderContext* renderer)
	: mRenderer(renderer)
	, mSpectral(new FrameBufferFloat(
		  renderer->spectrumDescriptor()->samples(),
		  renderer->width(),
		  renderer->height(),
		  0.0f))
{
	mIntCounter[V_Samples] = std::make_shared<FrameBufferUInt32>(
		renderer->spectrumDescriptor()->samples(),
		renderer->width(),
		renderer->height(),
		0);

	mIntCounter[V_Feedback] = std::make_shared<FrameBufferUInt32>(
		renderer->spectrumDescriptor()->samples(),
		renderer->width(),
		renderer->height(),
		0);
}

OutputBuffer::~OutputBuffer()
{
}

void OutputBuffer::clear()
{
	mSpectral->clear();

	for (uint32 i = 0; i < V_1D_COUNT; ++i) {
		if (mInt1D[i])
			mInt1D[i]->clear();
	}

	for (uint32 i = 0; i < V_COUNTER_COUNT; ++i) {
		if (mIntCounter[i])
			mIntCounter[i]->clear();
	}

	for (uint32 i = 0; i < V_3D_COUNT; ++i) {
		if (mInt3D[i])
			mInt3D[i]->clear();
	}

	for (auto p : mCustom1D)
		p.second->clear();

	for (auto p : mCustomCounter)
		p.second->clear();

	for (auto p : mCustom3D)
		p.second->clear();

	for (auto p : mCustomSpectral)
		p.second->clear();
}

#define _3D_S_B(ch, v)              \
	if (mInt3D[ch])                 \
		for (int i = 0; i < 3; ++i) \
	mInt3D[ch]->blendFragment(pixelIndex, i, v[i], ft)

#define _3D_S_L(ch, v)                                               \
	for (auto pair : mLPE_3D[ch]) {                                  \
		if (pair.first.match(path)) {                                \
			for (int i = 0; i < 3; ++i)                              \
				pair.second->blendFragment(pixelIndex, i, v[i], ft); \
		}                                                            \
	}

#define _3D_S(ch, v) \
	_3D_S_L(ch, v)   \
	_3D_S_B(ch, v)

#define _1D_S_B(ch, v) \
	if (mInt1D[ch])    \
	mInt1D[ch]->blendFragment(pixelIndex, 0, v, ft)

#define _1D_S_L(ch, v)                                        \
	for (auto pair : mLPE_1D[ch]) {                           \
		if (pair.first.match(path)) {                         \
			pair.second->blendFragment(pixelIndex, 0, v, ft); \
		}                                                     \
	}

#define _1D_S(ch, v) \
	_1D_S_L(ch, v)   \
	_1D_S_B(ch, v)

#define _C_S_B(ch, v)    \
	if (mIntCounter[ch]) \
	mIntCounter[ch]->blendFragment(pixelIndex, 0, v, ft)

#define _C_S_L(ch, v)                                         \
	for (auto pair : mLPE_Counter[ch]) {                      \
		if (pair.first.match(path)) {                         \
			pair.second->blendFragment(pixelIndex, 0, v, ft); \
		}                                                     \
	}

#define _C_S(ch, v) \
	_C_S_L(ch, v)   \
	_C_S_B(ch, v)

void OutputBuffer::pushFragment(uint32 pixelIndex, const ShadingPoint& s,
								const LightPath& path)
{
	const bool isInf	 = std::isinf(s.Radiance);
	const bool isNaN	 = std::isnan(s.Radiance);
	const bool isNeg	 = s.Radiance < 0;
	const bool isInvalid = isInf || isNaN || isNeg;

	const uint32 channel   = s.Ray.WavelengthIndex;
	const uint32 oldSample = getSampleCount(pixelIndex, channel);
	const float t		   = 1.0f / (oldSample + 1.0f);

	// Use accumulative sample count for special AOVs
	uint32 oldFullSample = 0;
	for (uint32 i = 0; i < mIntCounter[V_Samples]->channels(); ++i)
		oldFullSample += mIntCounter[V_Samples]->getFragment(pixelIndex, i);
	const float ft = 1.0f / (oldFullSample + 1.0f);

	if (!isInvalid)
		mSpectral->blendFragment(pixelIndex, channel, s.Radiance, t);

	_3D_S(V_Position, s.P);
	_3D_S(V_Normal, s.N);
	_3D_S(V_NormalG, s.Geometry.N);
	_3D_S(V_Tangent, s.Nx);
	_3D_S(V_Bitangent, s.Ny);
	_3D_S(V_View, s.Ray.Direction);
	_3D_S(V_UVW, s.Geometry.UVW);
	_3D_S(V_DPDT, s.Geometry.dPdT);

	_1D_S(V_Time, s.Ray.Time);
	_1D_S(V_Depth, std::sqrt(s.Depth2));
	_1D_S(V_Material, s.Geometry.MaterialID);

	// LPE
	if (!isInvalid) {
		for (auto pair : mLPE_Spectral) {
			if (pair.first.match(path)) {
				pair.second->blendFragment(pixelIndex, channel, s.Radiance, t);
			}
		}
	}

	if (isInvalid) {
		pushFeedbackFragment(pixelIndex, channel,
						  (isNaN ? OF_NaN : 0)
							  | (isInf ? OF_Infinite : 0)
							  | (isNeg ? OF_Negative : 0));
	}

	// Increase sample count
	incSampleCount(pixelIndex, channel);
}

void OutputBuffer::pushFeedbackFragment(uint32 pixelIndex, uint32 channel, uint32 feedback)
{
	PR_ASSERT(mIntCounter[V_Feedback], "Feedback buffer has to be available!");

	mIntCounter[V_Feedback]->getFragment(pixelIndex, channel) |= feedback;
}

void OutputBuffer::pushBackgroundFragment(uint32 pixelIndex, uint32 channel)
{
	incSampleCount(pixelIndex, channel);
}

} // namespace PR
