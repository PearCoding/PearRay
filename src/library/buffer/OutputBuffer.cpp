#include "OutputBuffer.h"
#include "Feedback.h"
#include "renderer/RenderContext.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
OutputBuffer::OutputBuffer(RenderContext* renderer)
	: mSpectral(new FrameBufferFloat(
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

#define _3D_S_B(ch, v, t)           \
	if (mInt3D[ch])                 \
		for (int i = 0; i < 3; ++i) \
	mInt3D[ch]->blendFragment(pixelIndex, i, v[i], t)

#define _3D_S_L(ch, v, t)                                           \
	for (auto pair : mLPE_3D[ch]) {                                 \
		if (pair.first.match(path)) {                               \
			for (int i = 0; i < 3; ++i)                             \
				pair.second->blendFragment(pixelIndex, i, v[i], t); \
		}                                                           \
	}

#define _3D_S(ch, v, t) \
	_3D_S_L(ch, v, t)   \
	_3D_S_B(ch, v, t)

#define _1D_S_B(ch, v, t) \
	if (mInt1D[ch])       \
	mInt1D[ch]->blendFragment(pixelIndex, 0, static_cast<float>(v), t)

#define _1D_S_L(ch, v, t)                                    \
	for (auto pair : mLPE_1D[ch]) {                          \
		if (pair.first.match(path)) {                        \
			pair.second->blendFragment(pixelIndex, 0, static_cast<float>(v), t); \
		}                                                    \
	}

#define _1D_S(ch, v, t) \
	_1D_S_L(ch, v, t)   \
	_1D_S_B(ch, v, t)

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

	if (!(s.Flags & SPF_Background)) {
		size_t sampleCount = 0;
		for (size_t i = 0; i < mIntCounter[V_Samples]->channels(); ++i)
			sampleCount = mIntCounter[V_Samples]->getFragment(pixelIndex, channel);
		float blend = 1.0f / (sampleCount + 1.0f);

		_3D_S(V_Position, s.P, blend);
		_3D_S(V_Normal, s.N, blend);
		_3D_S(V_NormalG, s.Geometry.N, blend);
		_3D_S(V_Tangent, s.Nx, blend);
		_3D_S(V_Bitangent, s.Ny, blend);
		_3D_S(V_View, s.Ray.Direction, blend);
		_3D_S(V_UVW, s.Geometry.UVW, blend);
		_3D_S(V_DPDT, s.Geometry.dPdT, blend);

		_1D_S(V_Time, s.Ray.Time, blend);
		_1D_S(V_Depth, std::sqrt(s.Depth2), blend);

		_1D_S(V_EntityID, s.EntityID, blend);
		_1D_S(V_MaterialID, s.Geometry.MaterialID, blend);
		_1D_S(V_EmissionID, s.Geometry.EmissionID, blend);
		_1D_S(V_DisplaceID, s.Geometry.DisplaceID, blend);
	}

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

	mIntCounter[V_Samples]->getFragment(pixelIndex, channel) += 1;
}

void OutputBuffer::pushFeedbackFragment(uint32 pixelIndex, uint32 channel, uint32 feedback)
{
	PR_ASSERT(mIntCounter[V_Feedback], "Feedback buffer has to be available!");

	mIntCounter[V_Feedback]->getFragment(pixelIndex, channel) |= feedback;
}

} // namespace PR
