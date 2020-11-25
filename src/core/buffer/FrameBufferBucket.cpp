#include "FrameBufferBucket.h"
#include "Feedback.h"
#include "filter/IFilter.h"
#include "output/OutputData.h"
#include "path/LightPathView.h"
#include "renderer/StreamPipeline.h"
#include "spectral/CIE.h"

namespace PR {
FrameBufferBucket::FrameBufferBucket(const std::shared_ptr<IFilter>& filter,
									 const Size2i& size, Size1i specChannels, bool monotonic)
	: mFilter(filter.get())
	, mOriginalSize(size)
	, mExtendedSize(mOriginalSize.Width + 2 * mFilter.radius(), mOriginalSize.Height + 2 * mFilter.radius())
	, mMonotonic(monotonic)
	, mHasFilter(mFilter.radius() > 0)
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

void FrameBufferBucket::commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entry_count)
{
	if (mMonotonic) {
		if (mHasFilter)
			commitSpectrals2<true, true>(pipeline, entries, entry_count);
		else
			commitSpectrals2<true, false>(pipeline, entries, entry_count);
	} else {
		if (mHasFilter)
			commitSpectrals2<false, true>(pipeline, entries, entry_count);
		else
			commitSpectrals2<false, false>(pipeline, entries, entry_count);
	}
}

template <bool IsMono, bool HasFilter>
void FrameBufferBucket::commitSpectrals2(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	const auto spectralCh  = mData.getInternalChannel_Spectral(AOV_Output);
	const auto weightCh	   = mData.getInternalChannel_1D(AOV_PixelWeight);
	auto varianceEstimator = mData.varianceEstimator();

	PR_ASSERT(HasFilter || filterRadius == 0, "If no filter is choosen, radius must be zero");

	const auto addContribution = [&](const Point2i& sp, float weight, const CIETriplet& triplet, const LightPathView& path) {
		float& weights = weightCh->getFragment(sp, 0);

		// Add contribution to main channel
		PR_UNROLL_LOOP(3)
		for (Size1i k = 0; k < 3; ++k)
			spectralCh->getFragment(sp, k) += triplet[k];

		// Add contribution to variance channel
		PR_UNROLL_LOOP(3)
		for (Size1i k = 0; k < 3; ++k)
			varianceEstimator.addValue(sp, k, weights, weight, triplet[k]);

		// Increment weights
		weights += weight;

		// LPE
		for (auto pair : mData.mLPE_Spectral[AOV_Output]) {
			if (pair.first.match(path)) {
				PR_UNROLL_LOOP(3)
				for (Size1i k = 0; k < 3; ++k)
					pair.second->getFragment(sp, k) += triplet[k];
			}
		}
	};

	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];

		const Point2i rp		   = entry.Position + filterSize;
		const bool isMono		   = IsMono || (entry.Flags & OSEF_Mono);
		const RayGroup& grp		   = pipeline->getRayGroup(entry.RayGroupID);
		const SpectralBlob factor  = isMono ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		const SpectralBlob contrib = factor * grp.BlendWeight * entry.contribution();
		const float blendWeight	   = (factor * grp.BlendWeight * entry.MIS).sum();

#ifndef PR_NO_SPECTRAL_CHECKS
		// Check for valid samples
		const bool isInf	 = contrib.isInf().any();
		const bool isNaN	 = contrib.isNaN().any();
		const bool isNeg	 = (contrib < 0.0f).any();
		const bool isInvalid = isInf | isNaN | isNeg;
		if (PR_UNLIKELY(isInvalid)) {
			const uint32 feedback = (isNaN ? OF_NaN : 0)
									| (isInf ? OF_Infinite : 0)
									| (isNeg ? OF_Negative : 0);

			mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(rp, 0) |= feedback;
			continue;
		}
#endif

		CIETriplet triplet;
		if constexpr (IsMono) {
			triplet = contrib[0] /* CIE::eval(entry.Wavelengths[0])*/;
		} else {
			// Map to CIE XYZ
			triplet = CIE::eval(contrib, entry.Wavelengths);
		}

		const LightPathView path = LightPathView(entry.Path);

		if constexpr (HasFilter) {
			// Apply for each filter area
			const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
			const Point2i end	= (extendedViewSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
			for (Point1i py = start(1); py <= end(1); ++py) {
				for (Point1i px = start(0); px <= end(0); ++px) {
					const Point2i sp		 = Point2i(px, py);
					const float filterWeight = mFilter.evalWeight(sp(0) - rp(0), sp(1) - rp(1));
					addContribution(sp, filterWeight * blendWeight, filterWeight * triplet, path);
				}
			}
		} else {
			addContribution(entry.Position, blendWeight, triplet, path);
		}
	}
}

#define BLEND_1D(var, val)                                   \
	if (mData.mInt1D[var]) {                                 \
		const auto var_e = mData.mInt1D[var];                \
		PR_OPT_LOOP                                          \
		for (size_t i = 0; i < entry_count; ++i) {           \
			const auto& entry = entries[i];                  \
			const Point2i sp  = entry.Position + filterSize; \
			var_e->getFragment(sp, 0) += val;                \
		}                                                    \
	}

#define BLEND_1D_LPE(var, val)                                    \
	for (auto pair : mData.mLPE_1D[var]) {                        \
		PR_OPT_LOOP                                               \
		for (size_t i = 0; i < entry_count; ++i) {                \
			const auto& entry		 = entries[i];                \
			const LightPathView path = LightPathView(entry.Path); \
			if (!pair.first.match(path))                          \
				continue;                                         \
			const Point2i sp = entry.Position + filterSize;       \
			pair.second->getFragment(sp, 0) += val;               \
		}                                                         \
	}

// Actually 3d by ignoring last channel
#define BLEND_2D(var, val)                                   \
	if (mData.mInt3D[var]) {                                 \
		const auto var_e = mData.mInt3D[var];                \
		PR_OPT_LOOP                                          \
		for (size_t i = 0; i < entry_count; ++i) {           \
			const auto& entry = entries[i];                  \
			const Point2i sp  = entry.Position + filterSize; \
			const Vector2f v  = val;                         \
			PR_UNROLL_LOOP(2)                                \
			for (Size1i k = 0; k < 2; ++k)                   \
				var_e->getFragment(sp, k) += v(k);           \
		}                                                    \
	}

#define BLEND_2D_LPE(var, val)                                    \
	for (auto pair : mData.mLPE_3D[var]) {                        \
		PR_OPT_LOOP                                               \
		for (size_t i = 0; i < entry_count; ++i) {                \
			const auto& entry		 = entries[i];                \
			const LightPathView path = LightPathView(entry.Path); \
			if (!pair.first.match(path))                          \
				continue;                                         \
			const Point2i sp = entry.Position + filterSize;       \
			const Vector2f v = val;                               \
			PR_UNROLL_LOOP(2)                                     \
			for (Size1i k = 0; k < 2; ++k)                        \
				pair.second->getFragment(sp, k) += v(k);          \
		}                                                         \
	}

#define BLEND_3D(var, val)                                   \
	if (mData.mInt3D[var]) {                                 \
		const auto var_e = mData.mInt3D[var];                \
		PR_OPT_LOOP                                          \
		for (size_t i = 0; i < entry_count; ++i) {           \
			const auto& entry = entries[i];                  \
			const Point2i sp  = entry.Position + filterSize; \
			const Vector3f v  = val;                         \
			PR_UNROLL_LOOP(3)                                \
			for (Size1i k = 0; k < 3; ++k)                   \
				var_e->getFragment(sp, k) += v(k);           \
		}                                                    \
	}

#define BLEND_3D_LPE(var, val)                                    \
	for (auto pair : mData.mLPE_3D[var]) {                        \
		PR_OPT_LOOP                                               \
		for (size_t i = 0; i < entry_count; ++i) {                \
			const auto& entry		 = entries[i];                \
			const LightPathView path = LightPathView(entry.Path); \
			if (!pair.first.match(path))                          \
				continue;                                         \
			const Point2i sp = entry.Position + filterSize;       \
			const Vector3f v = val;                               \
			PR_UNROLL_LOOP(3)                                     \
			for (Size1i k = 0; k < 3; ++k)                        \
				pair.second->getFragment(sp, k) += v(k);          \
		}                                                         \
	}

// TODO: Ignore medium shadingpoint entries
void FrameBufferBucket::commitShadingPoints(const OutputShadingPointEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	// Increase sample count
	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];
		const Point2i sp  = entry.Position + filterSize;
		mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0) += 1;
	}

	// Handle optional AOVs
	//BLEND_1D(AOV_Time, entry.SP.Ray.Time);
	BLEND_1D(AOV_Depth, std::sqrt(entry.SP.Depth2));

	BLEND_3D(AOV_Position, entry.SP.P);
	BLEND_3D(AOV_View, entry.SP.Ray.Direction);

	BLEND_1D(AOV_EntityID, entry.SP.Surface.Geometry.EntityID);
	BLEND_1D(AOV_MaterialID, entry.SP.Surface.Geometry.MaterialID);
	BLEND_1D(AOV_EmissionID, entry.SP.Surface.Geometry.EmissionID);
	BLEND_1D(AOV_DisplaceID, entry.SP.Surface.Geometry.DisplaceID);

	BLEND_3D(AOV_Normal, entry.SP.Surface.N);
	BLEND_3D(AOV_NormalG, entry.SP.Surface.Geometry.N);
	BLEND_3D(AOV_Tangent, entry.SP.Surface.Nx);
	BLEND_3D(AOV_Bitangent, entry.SP.Surface.Ny);
	BLEND_2D(AOV_UVW, entry.SP.Surface.Geometry.UV);
	// Check if there are any LPEs at all
	if (!mHasNonSpecLPE)
		return;

	// Handle optional LPE AOVs
	//BLEND_1D_LPE(AOV_Time, entry.SP.Ray.Time);
	BLEND_1D_LPE(AOV_Depth, std::sqrt(entry.SP.Depth2));
	BLEND_3D_LPE(AOV_Position, entry.SP.P);
	BLEND_3D_LPE(AOV_View, entry.SP.Ray.Direction);

	BLEND_1D_LPE(AOV_EntityID, entry.SP.Surface.Geometry.EntityID);
	BLEND_1D_LPE(AOV_MaterialID, entry.SP.Surface.Geometry.MaterialID);
	BLEND_1D_LPE(AOV_EmissionID, entry.SP.Surface.Geometry.EmissionID);
	BLEND_1D_LPE(AOV_DisplaceID, entry.SP.Surface.Geometry.DisplaceID);

	BLEND_3D_LPE(AOV_Normal, entry.SP.Surface.N);
	BLEND_3D_LPE(AOV_NormalG, entry.SP.Surface.Geometry.N);
	BLEND_3D_LPE(AOV_Tangent, entry.SP.Surface.Nx);
	BLEND_3D_LPE(AOV_Bitangent, entry.SP.Surface.Ny);
	BLEND_2D_LPE(AOV_UVW, entry.SP.Surface.Geometry.UV);
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
