#include "OutputBuffer.h"
#include "Feedback.h"
#include "shader/ShadingPoint.h"

namespace PR {
OutputBuffer::OutputBuffer(size_t width, size_t height, size_t specChannels)
	: mData(width, height, specChannels)
{
}

OutputBuffer::~OutputBuffer()
{
}

#define _3D_S_B(ch, v, t)           \
	if (mData.mInt3D[ch])           \
		for (int i = 0; i < 3; ++i) \
	mData.mInt3D[ch]->blendFragment(pixelIndex, i, v[i], t)

#define _3D_S_L(ch, v, t)                                           \
	for (auto pair : mData.mLPE_3D[ch]) {                           \
		if (pair.first.match(path)) {                               \
			for (int i = 0; i < 3; ++i)                             \
				pair.second->blendFragment(pixelIndex, i, v[i], t); \
		}                                                           \
	}

#define _3D_S(ch, v, t) \
	_3D_S_L(ch, v, t)   \
	_3D_S_B(ch, v, t)

#define _1D_S_B(ch, v, t) \
	if (mData.mInt1D[ch]) \
	mData.mInt1D[ch]->blendFragment(pixelIndex, 0, static_cast<float>(v), t)

#define _1D_S_L(ch, v, t)                                                        \
	for (auto pair : mData.mLPE_1D[ch]) {                                        \
		if (pair.first.match(path)) {                                            \
			pair.second->blendFragment(pixelIndex, 0, static_cast<float>(v), t); \
		}                                                                        \
	}

#define _1D_S(ch, v, t) \
	_1D_S_L(ch, v, t)   \
	_1D_S_B(ch, v, t)

void OutputBuffer::pushFragment(uint32 pixelIndex, const ShadingPoint& s,
								const LightPath& path)
{
	const bool isMono		 = (s.Ray.Flags & RF_Monochrome);
	const uint32 channels	= isMono ? 1 : 3;
	const uint32 monochannel = isMono ? s.Ray.WavelengthIndex : 0;

	bool isInf	 = false;
	bool isNaN	 = false;
	bool isNeg	 = false;
	bool isInvalid = false;
	for (uint32 i = 0; i < channels && !isInvalid; ++i) {
		isInf	 = std::isinf(s.Radiance[i]);
		isNaN	 = std::isnan(s.Radiance[i]);
		isNeg	 = s.Radiance[i] < 0;
		isInvalid = isInf || isNaN || isNeg;
	}

	if (!isInvalid) {
		for (uint32 i = 0; i < channels; ++i)
			mData.getInternalChannel_Spectral()->getFragment(pixelIndex, monochannel + i) += s.Ray.Weight[i] * s.Radiance[i];
	}

	if (!(s.Flags & SPF_Background)) {
		size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(pixelIndex, 0);
		float blend		   = 1.0f / (sampleCount + 1.0f);

		_3D_S(AOV_Position, s.P, blend);
		_3D_S(AOV_Normal, s.N, blend);
		_3D_S(AOV_NormalG, s.Geometry.N, blend);
		_3D_S(AOV_Tangent, s.Nx, blend);
		_3D_S(AOV_Bitangent, s.Ny, blend);
		_3D_S(AOV_View, s.Ray.Direction, blend);
		_3D_S(AOV_UVW, s.Geometry.UVW, blend);
		_3D_S(AOV_DPDT, s.Geometry.dPdT, blend);

		_1D_S(AOV_Time, s.Ray.Time, blend);
		_1D_S(AOV_Depth, std::sqrt(s.Depth2), blend);

		_1D_S(AOV_EntityID, s.EntityID, blend);
		_1D_S(AOV_MaterialID, s.Geometry.MaterialID, blend);
		_1D_S(AOV_EmissionID, s.Geometry.EmissionID, blend);
		_1D_S(AOV_DisplaceID, s.Geometry.DisplaceID, blend);
	}

	// LPE
	if (!isInvalid) {
		for (auto pair : mData.mLPE_Spectral) {
			if (pair.first.match(path)) {
				for (uint32 i = 0; i < channels; ++i) {
					pair.second->getFragment(pixelIndex, monochannel + i)
						+= s.Ray.Weight[i] * s.Radiance[i];
				}
			}
		}
	}

	if (isInvalid) {
		pushFeedbackFragment(pixelIndex,
							 (isNaN ? OF_NaN : 0)
								 | (isInf ? OF_Infinite : 0)
								 | (isNeg ? OF_Negative : 0));
	}

	mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(pixelIndex, 0) += 1;
}

void OutputBuffer::pushFeedbackFragment(uint32 pixelIndex, uint32 feedback)
{
	PR_ASSERT(mData.hasInternalChannel_Counter(AOV_Feedback), "Feedback buffer has to be available!");
	mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(pixelIndex, 0) |= feedback;
}

} // namespace PR
