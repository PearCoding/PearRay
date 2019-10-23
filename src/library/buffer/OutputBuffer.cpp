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
	mInt3D[ch]->getFragment(pixelIndex, i) += v[i]

#define _3D_S_L(ch, v)                                           \
	for (auto pair : mLPE_3D[ch]) {                              \
		if (pair.first.match(path)) {                            \
			for (int i = 0; i < 3; ++i)                          \
				pair.second->getFragment(pixelIndex, i) += v[i]; \
		}                                                        \
	}

#define _3D_S(ch, v) \
	_3D_S_L(ch, v)   \
	_3D_S_B(ch, v)

#define _1D_S_B(ch, v) \
	if (mInt1D[ch])    \
	mInt1D[ch]->getFragment(pixelIndex, 0) += v

#define _1D_S_L(ch, v)                                    \
	for (auto pair : mLPE_1D[ch]) {                       \
		if (pair.first.match(path)) {                     \
			pair.second->getFragment(pixelIndex, 0) += v; \
		}                                                 \
	}

#define _1D_S(ch, v) \
	_1D_S_L(ch, v)   \
	_1D_S_B(ch, v)

#define _C_S_B(ch, v)    \
	if (mIntCounter[ch]) \
	mIntCounter[ch]->getFragment(pixelIndex, 0) += v

#define _C_S_L(ch, v)                                     \
	for (auto pair : mLPE_Counter[ch]) {                  \
		if (pair.first.match(path)) {                     \
			pair.second->getFragment(pixelIndex, 0) += v; \
		}                                                 \
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

	const uint32 channel = s.Ray.WavelengthIndex;
	const float W		 = s.Ray.Weight;

	if (!isInvalid)
		mSpectral->getFragment(pixelIndex, channel) += W * s.Radiance;

	_3D_S(V_Position, W * s.P);
	_3D_S(V_Normal, W * s.N);
	_3D_S(V_NormalG, W * s.Geometry.N);
	_3D_S(V_Tangent, W * s.Nx);
	_3D_S(V_Bitangent, W * s.Ny);
	_3D_S(V_View, W * s.Ray.Direction);
	_3D_S(V_UVW, W * s.Geometry.UVW);
	_3D_S(V_DPDT, W * s.Geometry.dPdT);

	_1D_S(V_Time, W * s.Ray.Time);
	_1D_S(V_Depth, W * std::sqrt(s.Depth2));

	_1D_S(V_EntityID, W * s.EntityID);
	_1D_S(V_MaterialID, W * s.Geometry.MaterialID);
	_1D_S(V_EmissionID, W * s.Geometry.EmissionID);
	_1D_S(V_DisplaceID, W * s.Geometry.DisplaceID);

	// LPE
	if (!isInvalid) {
		for (auto pair : mLPE_Spectral) {
			if (pair.first.match(path)) {
				pair.second->getFragment(pixelIndex, channel) += W * s.Radiance;
			}
		}
	}

	if (isInvalid) {
		pushFeedbackFragment(pixelIndex, channel,
							 (isNaN ? OF_NaN : 0)
								 | (isInf ? OF_Infinite : 0)
								 | (isNeg ? OF_Negative : 0));
	}

	if (mIntCounter[V_Samples])
		mIntCounter[V_Samples]->getFragment(pixelIndex, channel) += 1;
}

void OutputBuffer::pushFeedbackFragment(uint32 pixelIndex, uint32 channel, uint32 feedback)
{
	PR_ASSERT(mIntCounter[V_Feedback], "Feedback buffer has to be available!");

	mIntCounter[V_Feedback]->getFragment(pixelIndex, channel) |= feedback;
}

void OutputBuffer::pushBackgroundFragment(uint32 pixelIndex, uint32 channel)
{
	if (mIntCounter[V_Samples])
		mIntCounter[V_Samples]->getFragment(pixelIndex, channel) += 1;
}

} // namespace PR
