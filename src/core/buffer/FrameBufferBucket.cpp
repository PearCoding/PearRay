#include "FrameBufferBucket.h"
#include "Feedback.h"
#include "filter/IFilter.h"
#include "output/OutputData.h"
#include "path/LightPathView.h"
#include "spectral/CIE.h"

// Uncomment this to allow all rays to contribute to AOVs outside spectrals
//#define PR_ALL_RAYS_CONTRIBUTE

namespace PR {
FrameBufferBucket::FrameBufferBucket(const std::shared_ptr<IFilter>& filter,
									 const Size2i& size, Size1i specChannels)
	: mFilter(filter.get())
	, mOriginalSize(size)
	, mExtendedSize(mOriginalSize.Width + 2 * mFilter.radius(), mOriginalSize.Height + 2 * mFilter.radius())
	, mViewSize(mOriginalSize)
	, mExtendedViewSize(mExtendedSize)
	, mData(mExtendedSize, specChannels)
	, mHasNonSpecLPE(false)
	, mSpectralMapBuffer{ std::vector<float>(specChannels), std::vector<float>(specChannels), std::vector<float>(specChannels) }
{
	PR_ASSERT(specChannels == 3, "Custom channel size not implemented yet :(");
}

FrameBufferBucket::~FrameBufferBucket()
{
}

void FrameBufferBucket::shrinkView(const Size2i& newView)
{
	PR_ASSERT(newView.Width <= mOriginalSize.Width, "Width greater then original");
	PR_ASSERT(newView.Height <= mOriginalSize.Height, "Height greater then original");

	mViewSize		  = newView;
	mExtendedViewSize = Size2i(newView.Width + 2 * mFilter.radius(), newView.Height + 2 * mFilter.radius());
}

void FrameBufferBucket::cache()
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

void FrameBufferBucket::commitSpectrals(const OutputSpectralEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];

		const Point2i rp  = entry.Position + filterSize;
		const bool isMono = entry.Flags & OSEF_Mono;

		SpectralBlob real_weight = entry.Weight;
		if (isMono) {
			PR_OPT_LOOP
			for (size_t k = 1; k < PR_SPECTRAL_BLOB_SIZE; ++k)
				real_weight[k] = 0;
		}

		const Size1i channels = isMono ? 1 : PR_SPECTRAL_BLOB_SIZE;

		bool isInf	   = false;
		bool isNaN	   = false;
		bool isNeg	   = false;
		bool isInvalid = false;
		for (Size1i i = 0; i < channels && !isInvalid; ++i) {
			isInf	  = std::isinf(real_weight[i]);
			isNaN	  = std::isnan(real_weight[i]);
			isNeg	  = real_weight[i] < 0;
			isInvalid = isInf || isNaN || isNeg;
		}

		if (!isInvalid) {
			CIETriplet triplet;
			CIE::eval(real_weight, entry.Wavelengths, triplet);
			const LightPathView path = LightPathView(entry.Path);

			const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
			const Point2i end	= (extendedViewSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
			for (Point1i py = start(1); py <= end(1); ++py) {
				for (Point1i px = start(0); px <= end(0); ++px) {
					Point2i sp				 = Point2i(px, py);
					const float filterWeight = mFilter.evalWeight(sp(0) - rp(0), sp(1) - rp(1));

					const CIETriplet weightedRad = filterWeight * triplet;
					for (Size1i i = 0; i < 3; ++i)
						mData.getInternalChannel_Spectral()->getFragment(sp, i) += weightedRad[i];

					// LPE
					for (auto pair : mData.mLPE_Spectral) {
						if (pair.first.match(path)) {
							for (Size1i i = 0; i < 3; ++i) {
								pair.second->getFragment(sp, i) += weightedRad[i];
							}
						}
					}
				}
			}
		} else {
			const uint32 feedback = (isNaN ? OF_NaN : 0)
									| (isInf ? OF_Infinite : 0)
									| (isNeg ? OF_Negative : 0);

			mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(rp, 0) |= feedback;
		}
	}
}

void FrameBufferBucket::commitShadingPoints(const OutputShadingPointEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	const auto blend1D = [&](const Point2i& ip, AOV1D var, float val, float blend) {
		if (mData.mInt1D[var])
			mData.mInt1D[var]->blendFragment(ip, 0, val, blend);
	};

	const auto blend1D_LPE = [&](const Point2i& ip, AOV1D var, float val, float blend, const LightPathView& path) {
		for (auto pair : mData.mLPE_1D[var]) {
			if (pair.first.match(path)) {
				pair.second->blendFragment(ip, 0, val, blend);
			}
		}
	};

	const auto blend3D = [&](const Point2i& ip, AOV3D var, const Vector3f& val, float blend) {
		if (mData.mInt3D[var])
			for (Size1i i = 0; i < 3; ++i)
				mData.mInt3D[var]->blendFragment(ip, i, val(i), blend);
	};

	const auto blend3D_LPE = [&](const Point2i& ip, AOV3D var, const Vector3f& val, float blend, const LightPathView& path) {
		for (auto pair : mData.mLPE_3D[var]) {
			if (pair.first.match(path)) {
				for (Size1i i = 0; i < 3; ++i)
					pair.second->blendFragment(ip, i, val(i), blend);
			}
		}
	};

	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];

#ifndef PR_ALL_RAYS_CONTRIBUTE
		if (entry.SP.Ray.IterationDepth != 0)
			continue;
#endif

		const Point2i sp	= entry.Position + filterSize;
		uint32& sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0);
		sampleCount += 1;

		const float blend = 1.0f / (sampleCount);

		blend3D(sp, AOV_Position, entry.SP.P, blend);
		blend3D(sp, AOV_Normal, entry.SP.N, blend);
		blend3D(sp, AOV_NormalG, entry.SP.Geometry.N, blend);
		blend3D(sp, AOV_Tangent, entry.SP.Nx, blend);
		blend3D(sp, AOV_Bitangent, entry.SP.Ny, blend);
		blend3D(sp, AOV_View, entry.SP.Ray.Direction, blend);
		blend3D(sp, AOV_UVW, entry.SP.Geometry.UVW, blend);
		blend3D(sp, AOV_DPDT, entry.SP.Geometry.dPdT, blend);

		blend1D(sp, AOV_Time, entry.SP.Ray.Time, blend);
		blend1D(sp, AOV_Depth, std::sqrt(entry.SP.Depth2), blend);

		blend1D(sp, AOV_EntityID, entry.SP.EntityID, blend);
		blend1D(sp, AOV_MaterialID, entry.SP.Geometry.MaterialID, blend);
		blend1D(sp, AOV_EmissionID, entry.SP.Geometry.EmissionID, blend);
		blend1D(sp, AOV_DisplaceID, entry.SP.Geometry.DisplaceID, blend);
	}

	if (mHasNonSpecLPE) {
		PR_OPT_LOOP
		for (size_t i = 0; i < entry_count; ++i) {
			const auto& entry = entries[i];

#ifndef PR_ALL_RAYS_CONTRIBUTE
			if (entry.SP.Ray.IterationDepth != 0)
				continue;
#endif

			const Point2i sp		 = entry.Position + filterSize;
			const uint32 sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0);
			const float blend		 = 1.0f / (sampleCount);

			const LightPathView path = LightPathView(entry.Path);

			blend3D_LPE(sp, AOV_Position, entry.SP.P, blend, path);
			blend3D_LPE(sp, AOV_Normal, entry.SP.N, blend, path);
			blend3D_LPE(sp, AOV_NormalG, entry.SP.Geometry.N, blend, path);
			blend3D_LPE(sp, AOV_Tangent, entry.SP.Nx, blend, path);
			blend3D_LPE(sp, AOV_Bitangent, entry.SP.Ny, blend, path);
			blend3D_LPE(sp, AOV_View, entry.SP.Ray.Direction, blend, path);
			blend3D_LPE(sp, AOV_UVW, entry.SP.Geometry.UVW, blend, path);
			blend3D_LPE(sp, AOV_DPDT, entry.SP.Geometry.dPdT, blend, path);

			blend1D_LPE(sp, AOV_Time, entry.SP.Ray.Time, blend, path);
			blend1D_LPE(sp, AOV_Depth, std::sqrt(entry.SP.Depth2), blend, path);

			blend1D_LPE(sp, AOV_EntityID, entry.SP.EntityID, blend, path);
			blend1D_LPE(sp, AOV_MaterialID, entry.SP.Geometry.MaterialID, blend, path);
			blend1D_LPE(sp, AOV_EmissionID, entry.SP.Geometry.EmissionID, blend, path);
			blend1D_LPE(sp, AOV_DisplaceID, entry.SP.Geometry.DisplaceID, blend, path);
		}
	}
}

void FrameBufferBucket::commitFeedbacks(const OutputFeedbackEntry* entries, size_t entry_count)
{
	PR_ASSERT(mData.hasInternalChannel_Counter(AOV_Feedback), "Feedback buffer has to be available!");
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];
		const Point2i sp  = entry.Position + filterSize;

		mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(sp, 0) |= entry.Feedback;
	}
}

} // namespace PR
