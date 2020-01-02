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
	, mHasNonSpecLPE(false)
{
}

OutputBufferBucket::~OutputBufferBucket()
{
}

void OutputBufferBucket::cache()
{
	mHasNonSpecLPE = false;
	for (int i = 0; i < AOV_1D_COUNT; ++i) {
		if (!mData.mLPE_1D[i].empty()) {
			mHasNonSpecLPE = true;
			break;
		}
	}
	if (!mHasNonSpecLPE) {
		for (int i = 0; i < AOV_3D_COUNT; ++i) {
			if (!mData.mLPE_3D[i].empty()) {
				mHasNonSpecLPE = true;
				break;
			}
		}
	}
	if (!mHasNonSpecLPE) {
		for (int i = 0; i < AOV_COUNTER_COUNT; ++i) {
			if (!mData.mLPE_Counter[i].empty()) {
				mHasNonSpecLPE = true;
				break;
			}
		}
	}
}

// Uncomment this to allow all rays to contribute to AOVs outside spectrals
//#define PR_ALL_RAYS_CONTRIBUTE

void OutputBufferBucket::pushSpectralFragment(uint32 x, uint32 y, const ColorTriplet& spec,
											  uint32 wavelengthIndex, bool isMono,
											  const LightPath& path)
{
	const int32 filterRadius = mFilter->radius();
	x += filterRadius;
	y += filterRadius;

	const uint32 channels	= isMono ? 1 : 3;
	const uint32 monochannel = isMono ? wavelengthIndex : 0;

	bool isInf	 = false;
	bool isNaN	 = false;
	bool isNeg	 = false;
	bool isInvalid = false;
	for (uint32 i = 0; i < channels && !isInvalid; ++i) {
		isInf	 = std::isinf(spec[i]);
		isNaN	 = std::isnan(spec[i]);
		isNeg	 = spec[i] < 0;
		isInvalid = isInf || isNaN || isNeg;
	}

	const uint32 sx = std::max<uint32>(0, (int32)x - (int32)filterRadius);
	const uint32 ex = std::min<uint32>(x + filterRadius, extendedWidth() - 1);
	const uint32 sy = std::max<uint32>(0, (int32)y - (int32)filterRadius);
	const uint32 ey = std::min<uint32>(y + filterRadius, extendedHeight() - 1);

	if (!isInvalid) {
		for (uint32 py = sy; py <= ey; ++py) {
			for (uint32 px = sx; px <= ex; ++px) {
				const float filterWeight = mFilter->evalWeight((int32)px - (int32)x,
															   (int32)py - (int32)y);

				const ColorTriplet weightedRad = filterWeight * spec;
				for (uint32 i = 0; i < channels; ++i)
					mData.getInternalChannel_Spectral()->getFragment(px, py, monochannel + i)
						+= weightedRad[i];

				// LPE
				for (auto pair : mData.mLPE_Spectral) {
					if (pair.first.match(path)) {
						for (uint32 i = 0; i < channels; ++i) {
							pair.second->getFragment(px, py, monochannel + i)
								+= weightedRad[i];
						}
					}
				}
			}
		}
	} else {
		pushFeedbackFragment(x - filterRadius, y - filterRadius,
							 (isNaN ? OF_NaN : 0)
								 | (isInf ? OF_Infinite : 0)
								 | (isNeg ? OF_Negative : 0));
	}
}

void OutputBufferBucket::pushSPFragment(uint32 x, uint32 y, const ShadingPoint& s,
										const LightPath& path)
{
	const int32 off = mFilter->radius();
	x += off;
	y += off;

	const auto blend1D = [&](uint32 ix, uint32 iy, AOV1D var, float val, float blend) {
		if (mData.mInt1D[var])
			mData.mInt1D[var]->blendFragment(ix, iy, 0, s.Ray.Weight[0] * val, blend);
	};

	const auto blend1D_LPE = [&](uint32 ix, uint32 iy, AOV1D var, float val, float blend) {
		for (auto pair : mData.mLPE_1D[var]) {
			if (pair.first.match(path)) {
				pair.second->blendFragment(ix, iy, 0, s.Ray.Weight[0] * val, blend);
			}
		}
	};

	const auto blend3D = [&](uint32 ix, uint32 iy, AOV3D var, const Vector3f& val, float blend) {
		if (mData.mInt3D[var])
			for (size_t i = 0; i < 3; ++i)
				mData.mInt3D[var]->blendFragment(ix, iy, i, s.Ray.Weight[i] * val(i), blend);
	};

	const auto blend3D_LPE = [&](uint32 ix, uint32 iy, AOV3D var, const Vector3f& val, float blend) {
		for (auto pair : mData.mLPE_3D[var]) {
			if (pair.first.match(path)) {
				for (size_t i = 0; i < 3; ++i)
					pair.second->blendFragment(ix, iy, i, s.Ray.Weight[i] * val(i), blend);
			}
		}
	};

#ifndef PR_ALL_RAYS_CONTRIBUTE
	if (s.Ray.IterationDepth == 0) {
#endif
		size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(x, y, 0);
		float blend		   = 1.0f / (sampleCount + 1.0f);

		blend3D(x, y, AOV_Position, s.P, blend);
		blend3D(x, y, AOV_Normal, s.N, blend);
		blend3D(x, y, AOV_NormalG, s.Geometry.N, blend);
		blend3D(x, y, AOV_Tangent, s.Nx, blend);
		blend3D(x, y, AOV_Bitangent, s.Ny, blend);
		blend3D(x, y, AOV_View, s.Ray.Direction, blend);
		blend3D(x, y, AOV_UVW, s.Geometry.UVW, blend);
		blend3D(x, y, AOV_DPDT, s.Geometry.dPdT, blend);

		blend1D(x, y, AOV_Time, s.Ray.Time, blend);
		blend1D(x, y, AOV_Depth, std::sqrt(s.Depth2), blend);

		blend1D(x, y, AOV_EntityID, s.EntityID, blend);
		blend1D(x, y, AOV_MaterialID, s.Geometry.MaterialID, blend);
		blend1D(x, y, AOV_EmissionID, s.Geometry.EmissionID, blend);
		blend1D(x, y, AOV_DisplaceID, s.Geometry.DisplaceID, blend);

		mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(x, y, 0) += 1;

#ifndef PR_ALL_RAYS_CONTRIBUTE
	}
#endif

	if (mHasNonSpecLPE) {
		size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(x, y, 0);
		float blend		   = 1.0f / (sampleCount + 1.0f);

		blend3D_LPE(x, y, AOV_Position, s.P, blend);
		blend3D_LPE(x, y, AOV_Normal, s.N, blend);
		blend3D_LPE(x, y, AOV_NormalG, s.Geometry.N, blend);
		blend3D_LPE(x, y, AOV_Tangent, s.Nx, blend);
		blend3D_LPE(x, y, AOV_Bitangent, s.Ny, blend);
		blend3D_LPE(x, y, AOV_View, s.Ray.Direction, blend);
		blend3D_LPE(x, y, AOV_UVW, s.Geometry.UVW, blend);
		blend3D_LPE(x, y, AOV_DPDT, s.Geometry.dPdT, blend);

		blend1D_LPE(x, y, AOV_Time, s.Ray.Time, blend);
		blend1D_LPE(x, y, AOV_Depth, std::sqrt(s.Depth2), blend);

		blend1D_LPE(x, y, AOV_EntityID, s.EntityID, blend);
		blend1D_LPE(x, y, AOV_MaterialID, s.Geometry.MaterialID, blend);
		blend1D_LPE(x, y, AOV_EmissionID, s.Geometry.EmissionID, blend);
		blend1D_LPE(x, y, AOV_DisplaceID, s.Geometry.DisplaceID, blend);
	}
}

void OutputBufferBucket::pushFeedbackFragment(uint32 x, uint32 y, uint32 feedback)
{
	const size_t off = mFilter->radius();

	PR_ASSERT(mData.hasInternalChannel_Counter(AOV_Feedback), "Feedback buffer has to be available!");
	mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(x + off, y + off, 0) |= feedback;
}

} // namespace PR
