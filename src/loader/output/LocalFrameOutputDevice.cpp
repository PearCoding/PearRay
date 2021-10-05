#include "LocalFrameOutputDevice.h"
#include "filter/IFilter.h"
#include "output/Feedback.h"
#include "output/OutputData.h"
#include "path/LightPathView.h"
#include "renderer/StreamPipeline.h"
#include "spectral/CIE.h"

namespace PR {
LocalFrameOutputDevice::LocalFrameOutputDevice(const std::shared_ptr<IFilter>& filter,
											   const Size2i& size, Size1i specChannels, bool monotonic)
	: LocalOutputDevice()
	, mFilter(filter.get())
	, mOriginalSize(size)
	, mExtendedSize(mOriginalSize.Width + 2 * mFilter.radius(), mOriginalSize.Height + 2 * mFilter.radius())
	, mMonotonic(monotonic)
	, mHasFilter(mFilter.radius() > 0)
	, mData(mExtendedSize, specChannels)
	, mHasNonSpecLPE(false)
	, mSpectralMapBuffer{ std::vector<float>(specChannels), std::vector<float>(specChannels), std::vector<float>(specChannels) }
{
	PR_ASSERT(specChannels == 3, "Custom channel size not implemented yet :(");
}

LocalFrameOutputDevice::~LocalFrameOutputDevice()
{
}

void LocalFrameOutputDevice::clear(bool force)
{
	mData.clear(force);
}

void LocalFrameOutputDevice::cache()
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

void LocalFrameOutputDevice::commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entry_count)
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

template <bool IsMono>
static inline CIETriplet mapSpectral(const SpectralBlob& weight, const SpectralBlob& wvls)
{
	if constexpr (IsMono) {
		PR_UNUSED(wvls);
		return CIETriplet(weight[0]);
	} else {
		return CIE::eval(weight, wvls);
	}
}

template <bool IsMono, bool HasFilter>
void LocalFrameOutputDevice::commitSpectrals2(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	const auto spectralCh = mData.getInternalChannel_Spectral(AOV_Output);

	PR_ASSERT(HasFilter || filterRadius == 0, "If no filter is choosen, radius must be zero");

	const auto addContribution = [&](const Point2i& sp, const CIETriplet& triplet, const LightPathView& path) {
		// Add contribution to main channel
		PR_UNROLL_LOOP(3)
		for (Size1i k = 0; k < 3; ++k)
			spectralCh->getFragment(sp, k) += triplet[k];

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

		const Point2i rp			  = entry.Position + filterSize;
		const bool isMono			  = IsMono || (entry.Flags & OutputSpectralEntryFlag::Mono);
		const SpectralBlob heroFactor = isMono ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		const RayGroup& grp			  = pipeline->getRayGroup(entry.RayGroupID);
		const SpectralBlob contrib	  = heroFactor * entry.contribution();

		const SpectralBlob wvls = grp.WavelengthNM;

#ifndef PR_NO_SPECTRAL_CHECKS
		// Check for valid samples
		const bool isInf	 = contrib.isInf().any();
		const bool isNaN	 = contrib.isNaN().any();
		const bool isNeg	 = (contrib < -PR_EPSILON).any();
		const bool isInvalid = isInf | isNaN | isNeg;
		if (PR_UNLIKELY(isInvalid)) {
			OutputFeedbackFlags feedback = 0;
			if (isNaN)
				feedback |= OutputFeedback::NaN;
			if (isInf)
				feedback |= OutputFeedback::Infinite;
			if (isNeg)
				feedback |= OutputFeedback::Negative;

			mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(rp, 0) |= feedback;
			continue;
		}
#endif

		const CIETriplet triplet = mapSpectral<IsMono>(contrib, wvls);
		const LightPathView path = LightPathView(entry.Path);

		if constexpr (HasFilter) {
			// Apply for each filter area
			const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
			const Point2i end	= (extendedSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
			for (Point1i py = start(1); py <= end(1); ++py) {
				for (Point1i px = start(0); px <= end(0); ++px) {
					const Point2i sp		 = Point2i(px, py);
					const float filterWeight = mFilter.evalWeight(sp(0) - rp(0), sp(1) - rp(1));
					if (filterWeight > PR_EPSILON)
						addContribution(sp, filterWeight * grp.BlendWeight * triplet, path);
				}
			}
		} else {
			addContribution(entry.Position, grp.BlendWeight * triplet, path);
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
void LocalFrameOutputDevice::commitShadingPoints(const OutputShadingPointEntry* entries, size_t entry_count)
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

void LocalFrameOutputDevice::commitFeedbacks(const OutputFeedbackEntry* entries, size_t entry_count)
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

void LocalFrameOutputDevice::commitCustomSpectrals(uint32 aov_id, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entry_count)
{
	const auto aov = mData.getCustomChannel_Spectral(aov_id);
	if (!aov)
		return;

	if (mMonotonic) {
		if (mHasFilter)
			commitCustomSpectrals2<true, true>(aov.get(), pipeline, entries, entry_count);
		else
			commitCustomSpectrals2<true, false>(aov.get(), pipeline, entries, entry_count);
	} else {
		if (mHasFilter)
			commitCustomSpectrals2<false, true>(aov.get(), pipeline, entries, entry_count);
		else
			commitCustomSpectrals2<false, false>(aov.get(), pipeline, entries, entry_count);
	}
}

template <bool IsMono, bool HasFilter>
void LocalFrameOutputDevice::commitCustomSpectrals2(FrameBufferFloat* aov, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entry_count)
{
	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_ASSERT(HasFilter || filterRadius == 0, "If no filter is choosen, radius must be zero");

	const auto addContribution = [&](const Point2i& sp, float weight, const CIETriplet& triplet) {
		// Add contribution to main channel
		PR_UNROLL_LOOP(3)
		for (Size1i k = 0; k < 3; ++k)
			aov->getFragment(sp, k) += weight * triplet[k];
	};

	PR_OPT_LOOP
	for (size_t i = 0; i < entry_count; ++i) {
		const auto& entry = entries[i];

		const Point2i rp		   = entry.Position + filterSize;
		const bool isMono		   = IsMono || (entry.Flags & OutputSpectralEntryFlag::Mono);
		const RayGroup& grp		   = pipeline->getRayGroup(entry.RayGroupID);
		const SpectralBlob factor  = isMono ? SpectralBlobUtils::HeroOnly() : SpectralBlob::Ones();
		const SpectralBlob contrib = factor * grp.BlendWeight * entry.Value;
		const float iterWeight	   = grp.BlendWeight;

		const CIETriplet triplet = mapSpectral<IsMono>(contrib, entry.Wavelengths);

		if constexpr (HasFilter) {
			// Apply for each filter area
			const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
			const Point2i end	= (extendedSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
			for (Point1i py = start(1); py <= end(1); ++py) {
				for (Point1i px = start(0); px <= end(0); ++px) {
					const Point2i sp		 = Point2i(px, py);
					const float filterWeight = mFilter.evalWeight(sp(0) - rp(0), sp(1) - rp(1));
					addContribution(sp, filterWeight * iterWeight, filterWeight * triplet);
				}
			}
		} else {
			addContribution(entry.Position, iterWeight, triplet);
		}
	}
}

void LocalFrameOutputDevice::commitCustom3D(uint32 aov_id, const OutputCustom3DEntry* entries, size_t entrycount)
{
	const auto aov = mData.getCustomChannel_3D(aov_id);
	if (!aov)
		return;

	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_OPT_LOOP
	for (size_t i = 0; i < entrycount; ++i) {
		const auto& entry = entries[i];
		const Point2i sp  = entry.Position + filterSize;
		PR_UNROLL_LOOP(3)
		for (Size1i k = 0; k < 3; ++k)
			aov->getFragment(sp, k) += entry.Value(k);
	}
}

void LocalFrameOutputDevice::commitCustom1D(uint32 aov_id, const OutputCustom1DEntry* entries, size_t entrycount)
{
	const auto aov = mData.getCustomChannel_1D(aov_id);
	if (!aov)
		return;

	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_OPT_LOOP
	for (size_t i = 0; i < entrycount; ++i) {
		const auto& entry = entries[i];
		const Point2i sp  = entry.Position + filterSize;
		aov->getFragment(sp, 0) += entry.Value;
	}
}
void LocalFrameOutputDevice::commitCustomCounter(uint32 aov_id, const OutputCustomCounterEntry* entries, size_t entrycount)
{
	const auto aov = mData.getCustomChannel_Counter(aov_id);
	if (!aov)
		return;

	const int32 filterRadius = mFilter.radius();
	const Size2i filterSize	 = Size2i(filterRadius, filterRadius);

	PR_OPT_LOOP
	for (size_t i = 0; i < entrycount; ++i) {
		const auto& entry = entries[i];
		const Point2i sp  = entry.Position + filterSize;
		aov->getFragment(sp, 0) += entry.Value;
	}
}
} // namespace PR
