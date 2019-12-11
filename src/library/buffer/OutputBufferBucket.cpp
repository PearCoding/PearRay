#include "OutputBufferBucket.h"
#include "Feedback.h"
#include "filter/IFilter.h"
#include "shader/ShadingPoint.h"

namespace PR {
OutputBufferBucket::OutputBufferBucket(const std::shared_ptr<IFilter>& filter,
									   size_t width, size_t height, size_t specChannels)
	: mFilter(filter)
	, mOriginalWidth(width)
	, mOriginalHeight(height)
	, mExtendedWidth(width + 2 * mFilter->radius())
	, mExtendedHeight(height + 2 * mFilter->radius())
	, mData(mExtendedWidth, mExtendedHeight, specChannels)
{
}

OutputBufferBucket::~OutputBufferBucket()
{
}

void OutputBufferBucket::pushFragment(uint32 x, uint32 y, const ShadingPoint& s,
									  const LightPath& path)
{
	const size_t filterRadius = mFilter->radius();

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

	const auto blend1D = [&](uint32 ix, uint32 iy, AOV1D var, float val, float blend) {
		if (mData.mInt1D[var])
			mData.mInt1D[var]->blendFragment(ix, iy, 0, val, blend);

		for (auto pair : mData.mLPE_1D[var]) {
			if (pair.first.match(path)) {
				pair.second->blendFragment(ix, iy, 0, val, blend);
			}
		}
	};

	const auto blend3D = [&](uint32 ix, uint32 iy, AOV3D var, const Vector3f& val, float blend) {
		if (mData.mInt3D[var])
			for (size_t i = 0; i < 3; ++i)
				mData.mInt3D[var]->blendFragment(ix, iy, i, val(i), blend);

		for (auto pair : mData.mLPE_3D[var]) {
			if (pair.first.match(path)) {
				for (size_t i = 0; i < 3; ++i)
					pair.second->blendFragment(ix, iy, i, val(i), blend);
			}
		}
	};

	for (uint32 py = y - filterRadius; py <= y + filterRadius; ++py) {
		for (uint32 px = x - filterRadius; px <= x + filterRadius; ++px) {
			const float filterWeight = mFilter->evalWeight((int32)px - (int32)x, (int32)py - (int32)y);

			if (!isInvalid) {
				for (uint32 i = 0; i < channels; ++i)
					mData.getInternalChannel_Spectral()->getFragment(px, py, monochannel + i)
						+= s.Ray.Weight[i] * s.Radiance[i] * filterWeight;
			}

			if (!(s.Flags & SPF_Background)) {
				size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(px, py, 0);
				float blend		   = filterWeight * 1.0f / (sampleCount + 1.0f);

				blend3D(px, py, AOV_Position, s.P, blend);
				blend3D(px, py, AOV_Normal, s.N, blend);
				blend3D(px, py, AOV_NormalG, s.Geometry.N, blend);
				blend3D(px, py, AOV_Tangent, s.Nx, blend);
				blend3D(px, py, AOV_Bitangent, s.Ny, blend);
				blend3D(px, py, AOV_View, s.Ray.Direction, blend);
				blend3D(px, py, AOV_UVW, s.Geometry.UVW, blend);
				blend3D(px, py, AOV_DPDT, s.Geometry.dPdT, blend);

				blend1D(px, py, AOV_Time, s.Ray.Time, blend);
				blend1D(px, py, AOV_Depth, std::sqrt(s.Depth2), blend);

				blend1D(px, py, AOV_EntityID, s.EntityID, blend);
				blend1D(px, py, AOV_MaterialID, s.Geometry.MaterialID, blend);
				blend1D(px, py, AOV_EmissionID, s.Geometry.EmissionID, blend);
				blend1D(px, py, AOV_DisplaceID, s.Geometry.DisplaceID, blend);
			}

			// LPE
			if (!isInvalid) {
				for (auto pair : mData.mLPE_Spectral) {
					if (pair.first.match(path)) {
						for (uint32 i = 0; i < channels; ++i) {
							pair.second->getFragment(px, py, monochannel + i)
								+= filterWeight * s.Ray.Weight[i] * s.Radiance[i];
						}
					}
				}
			}

			mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(px, py, 0) += 1;
		}
	}

	// Independent of filter
	if (isInvalid) {
		pushFeedbackFragment(x, y,
							 (isNaN ? OF_NaN : 0)
								 | (isInf ? OF_Infinite : 0)
								 | (isNeg ? OF_Negative : 0));
	}
}

void OutputBufferBucket::pushFeedbackFragment(uint32 x, uint32 y, uint32 feedback)
{
	const size_t offX = (mExtendedWidth - mOriginalWidth) / 2;
	const size_t offY = (mExtendedHeight - mOriginalHeight) / 2;

	PR_ASSERT(mData.hasInternalChannel_Counter(AOV_Feedback), "Feedback buffer has to be available!");
	mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(x + offX, y + offY, 0) |= feedback;
}

} // namespace PR
