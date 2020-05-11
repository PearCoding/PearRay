#include "OutputBufferBucket.h"
#include "Feedback.h"
#include "filter/IFilter.h"
#include "shader/ShadingPoint.h"

namespace PR {
OutputBufferBucket::OutputBufferBucket(const std::shared_ptr<IFilter>& filter,
									   const Size2i& size, Size1i specChannels)
	: mFilter(filter)
	, mOriginalSize(size)
	, mExtendedSize(mOriginalSize.Width + 2 * mFilter->radius(), mOriginalSize.Height + 2 * mFilter->radius())
	, mViewSize(mOriginalSize)
	, mExtendedViewSize(mExtendedSize)
	, mData(mExtendedSize, specChannels)
	, mHasNonSpecLPE(false)
	, mSpectralMapBuffer{ std::vector<float>(specChannels), std::vector<float>(specChannels), std::vector<float>(specChannels) }
{
}

OutputBufferBucket::~OutputBufferBucket()
{
}

void OutputBufferBucket::shrinkView(const Size2i& newView)
{
	PR_ASSERT(newView.Width <= mOriginalSize.Width, "Width greater then original");
	PR_ASSERT(newView.Height <= mOriginalSize.Height, "Height greater then original");

	mViewSize		  = newView;
	mExtendedViewSize = Size2i(newView.Width + 2 * mFilter->radius(), newView.Height + 2 * mFilter->radius());
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

void OutputBufferBucket::pushSpectralFragment(const Point2i& p, const SpectralBlob& spec,
											  uint32 wavelengthIndex, bool isMono,
											  const LightPath& path)
{
	const int32 filterRadius = mFilter->radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);
	const Point2i rp		 = p + filterSize;

	const Size1i channels	 = isMono ? 1 : 3 /*SPECTRAL_BLOB_SIZE*/;
	const Size1i monochannel = isMono ? wavelengthIndex : 0;

	bool isInf	   = false;
	bool isNaN	   = false;
	bool isNeg	   = false;
	bool isInvalid = false;
	for (Size1i i = 0; i < channels && !isInvalid; ++i) {
		isInf	  = std::isinf(spec[i]);
		isNaN	  = std::isnan(spec[i]);
		isNeg	  = spec[i] < 0;
		isInvalid = isInf || isNaN || isNeg;
	}

	if (!isInvalid) {
		const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
		const Point2i end	= (extendedViewSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
		for (Point1i py = start(1); py <= end(1); ++py) {
			for (Point1i px = start(0); px <= end(0); ++px) {
				Point2i sp				 = Point2i(px, py);
				const float filterWeight = mFilter->evalWeight(sp(0) - rp(0), sp(1) - rp(1));

				// TODO Map input color triplet to spectral buffer and than push to spectral frame buffer!
				const SpectralBlob weightedRad = filterWeight * spec;
				for (Size1i i = 0; i < channels; ++i)
					mData.getInternalChannel_Spectral()->getFragment(sp, monochannel + i)
						+= weightedRad[i];

				// LPE
				for (auto pair : mData.mLPE_Spectral) {
					if (pair.first.match(path)) {
						for (Size1i i = 0; i < channels; ++i) {
							pair.second->getFragment(sp, monochannel + i)
								+= weightedRad[i];
						}
					}
				}
			}
		}
	} else {
		pushFeedbackFragment(p,
							 (isNaN ? OF_NaN : 0)
								 | (isInf ? OF_Infinite : 0)
								 | (isNeg ? OF_Negative : 0));
	}
}

void OutputBufferBucket::pushSPFragment(const Point2i& p, const ShadingPoint& s,
										const LightPath& path)
{
	const int32 filterRadius = mFilter->radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);
	const Point2i sp		 = p + filterSize;

	const auto blend1D = [&](const Point2i& ip, AOV1D var, float val, float blend) {
		if (mData.mInt1D[var])
			mData.mInt1D[var]->blendFragment(ip, 0, s.Ray.Weight[0] * val, blend);
	};

	const auto blend1D_LPE = [&](const Point2i& ip, AOV1D var, float val, float blend) {
		for (auto pair : mData.mLPE_1D[var]) {
			if (pair.first.match(path)) {
				pair.second->blendFragment(ip, 0, s.Ray.Weight[0] * val, blend);
			}
		}
	};

	const auto blend3D = [&](const Point2i& ip, AOV3D var, const Vector3f& val, float blend) {
		if (mData.mInt3D[var])
			for (Size1i i = 0; i < 3; ++i)
				mData.mInt3D[var]->blendFragment(ip, i, s.Ray.Weight[i] * val(i), blend);
	};

	const auto blend3D_LPE = [&](const Point2i& ip, AOV3D var, const Vector3f& val, float blend) {
		for (auto pair : mData.mLPE_3D[var]) {
			if (pair.first.match(path)) {
				for (Size1i i = 0; i < 3; ++i)
					pair.second->blendFragment(ip, i, s.Ray.Weight[i] * val(i), blend);
			}
		}
	};

#ifndef PR_ALL_RAYS_CONTRIBUTE
	if (s.Ray.IterationDepth == 0) {
#endif
		size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0);
		float blend		   = 1.0f / (sampleCount + 1.0f);

		blend3D(sp, AOV_Position, s.P, blend);
		blend3D(sp, AOV_Normal, s.N, blend);
		blend3D(sp, AOV_NormalG, s.Geometry.N, blend);
		blend3D(sp, AOV_Tangent, s.Nx, blend);
		blend3D(sp, AOV_Bitangent, s.Ny, blend);
		blend3D(sp, AOV_View, s.Ray.Direction, blend);
		blend3D(sp, AOV_UVW, s.Geometry.UVW, blend);
		blend3D(sp, AOV_DPDT, s.Geometry.dPdT, blend);

		blend1D(sp, AOV_Time, s.Ray.Time, blend);
		blend1D(sp, AOV_Depth, std::sqrt(s.Depth2), blend);

		blend1D(sp, AOV_EntityID, s.EntityID, blend);
		blend1D(sp, AOV_MaterialID, s.Geometry.MaterialID, blend);
		blend1D(sp, AOV_EmissionID, s.Geometry.EmissionID, blend);
		blend1D(sp, AOV_DisplaceID, s.Geometry.DisplaceID, blend);

		mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0) += 1;

#ifndef PR_ALL_RAYS_CONTRIBUTE
	}
#endif

	if (mHasNonSpecLPE) {
		size_t sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0);
		float blend		   = 1.0f / (sampleCount + 1.0f);

		blend3D_LPE(sp, AOV_Position, s.P, blend);
		blend3D_LPE(sp, AOV_Normal, s.N, blend);
		blend3D_LPE(sp, AOV_NormalG, s.Geometry.N, blend);
		blend3D_LPE(sp, AOV_Tangent, s.Nx, blend);
		blend3D_LPE(sp, AOV_Bitangent, s.Ny, blend);
		blend3D_LPE(sp, AOV_View, s.Ray.Direction, blend);
		blend3D_LPE(sp, AOV_UVW, s.Geometry.UVW, blend);
		blend3D_LPE(sp, AOV_DPDT, s.Geometry.dPdT, blend);

		blend1D_LPE(sp, AOV_Time, s.Ray.Time, blend);
		blend1D_LPE(sp, AOV_Depth, std::sqrt(s.Depth2), blend);

		blend1D_LPE(sp, AOV_EntityID, s.EntityID, blend);
		blend1D_LPE(sp, AOV_MaterialID, s.Geometry.MaterialID, blend);
		blend1D_LPE(sp, AOV_EmissionID, s.Geometry.EmissionID, blend);
		blend1D_LPE(sp, AOV_DisplaceID, s.Geometry.DisplaceID, blend);
	}
}

void OutputBufferBucket::pushFeedbackFragment(const Point2i& p, uint32 feedback)
{
	const int32 filterRadius = mFilter->radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);
	const Point2i sp		 = p + filterSize;

	PR_ASSERT(mData.hasInternalChannel_Counter(AOV_Feedback), "Feedback buffer has to be available!");
	mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(sp, 0) |= feedback;
}

} // namespace PR
