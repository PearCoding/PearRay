#include "FrameBufferBucket.h"
#include "Feedback.h"
#include "filter/IFilter.h"
#include "output/OutputData.h"
#include "path/LightPathView.h"
#include "spectral/CIE.h"

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
		/*Size1i w_channels			 = 0;
		for(size_t k = 0; k <PR_SPECTRAL_BLOB_SIZE; ++k) {
			if(entry.Weight[k] > PR_EPSILON)
				++w_channels;
		}*/

		//real_weight *= w_channels/(float)PR_SPECTRAL_BLOB_SIZE;
		size_t channels = PR_SPECTRAL_BLOB_SIZE;
		if (PR_UNLIKELY(isMono)) {
			channels = 1;
			real_weight *= SpectralBlobUtils::HeroOnly() * PR_SPECTRAL_BLOB_SIZE;
		}

		// Check for valid samples
		bool isInf	   = false;
		bool isNaN	   = false;
		bool isNeg	   = false;
		bool isInvalid = false;
		for (size_t k = 0; k < channels && !isInvalid; ++k) {
			isInf	  = std::isinf(real_weight[k]);
			isNaN	  = std::isnan(real_weight[k]);
			isNeg	  = real_weight[k] < 0;
			isInvalid = isInf || isNaN || isNeg;
		}
		if (PR_UNLIKELY(isInvalid)) {
			const uint32 feedback = (isNaN ? OF_NaN : 0)
									| (isInf ? OF_Infinite : 0)
									| (isNeg ? OF_Negative : 0);

			mData.getInternalChannel_Counter(AOV_Feedback)->getFragment(rp, 0) |= feedback;
			continue;
		}

		// Map to CIE XYZ
		CIETriplet triplet;
		CIE::eval(real_weight, entry.Wavelengths, triplet);
		const LightPathView path = LightPathView(entry.Path);

		// Apply for each filter area
		const Point2i start = Point2i::Zero().cwiseMax(rp - filterSize);
		const Point2i end	= (extendedViewSize() - Point2i(1, 1)).cwiseMin(rp + filterSize);
		for (Point1i py = start(1); py <= end(1); ++py) {
			for (Point1i px = start(0); px <= end(0); ++px) {
				const Point2i sp			 = Point2i(px, py);
				const float filterWeight	 = mFilter.evalWeight(sp(0) - rp(0), sp(1) - rp(1));
				const CIETriplet weightedRad = filterWeight * triplet;

				PR_UNROLL_LOOP(3)
				for (Size1i k = 0; k < 3; ++k)
					mData.getInternalChannel_Spectral()->getFragment(sp, k) += weightedRad[k];

				// LPE
				for (auto pair : mData.mLPE_Spectral) {
					if (pair.first.match(path)) {
						PR_UNROLL_LOOP(3)
						for (Size1i k = 0; k < 3; ++k)
							pair.second->getFragment(sp, k) += weightedRad[k];
					}
				}
			}
		}
	}
}

#define BLEND_1D(var, val)                                                                                    \
	if (mData.mInt1D[var]) {                                                                                  \
		const auto var_e = mData.mInt1D[var];                                                                 \
		PR_OPT_LOOP                                                                                           \
		for (size_t i = 0; i < entry_count; ++i) {                                                            \
			const auto& entry		 = entries[i];                                                            \
			const Point2i sp		 = entry.Position + filterSize;                                           \
			const uint32 sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0); \
			const float blend		 = 1.0f / (sampleCount);                                                  \
			var_e->blendFragment(sp, 0, entry.SP.Ray.Weight[0] * (val), blend);                               \
		}                                                                                                     \
	}

#define BLEND_1D_LPE(var, val)                                                                                \
	for (auto pair : mData.mLPE_1D[var]) {                                                                    \
		PR_OPT_LOOP                                                                                           \
		for (size_t i = 0; i < entry_count; ++i) {                                                            \
			const auto& entry		 = entries[i];                                                            \
			const LightPathView path = LightPathView(entry.Path);                                             \
			if (!pair.first.match(path))                                                                      \
				continue;                                                                                     \
			const Point2i sp		 = entry.Position + filterSize;                                           \
			const uint32 sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0); \
			const float blend		 = 1.0f / (sampleCount);                                                  \
			pair.second->blendFragment(sp, 0, entry.SP.Ray.Weight[0] * (val), blend);                         \
		}                                                                                                     \
	}

#define BLEND_3D(var, val)                                                                                    \
	if (mData.mInt3D[var]) {                                                                                  \
		const auto var_e = mData.mInt3D[var];                                                                 \
		PR_OPT_LOOP                                                                                           \
		for (size_t i = 0; i < entry_count; ++i) {                                                            \
			const auto& entry		 = entries[i];                                                            \
			const Point2i sp		 = entry.Position + filterSize;                                           \
			const uint32 sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0); \
			const float blend		 = 1.0f / (sampleCount);                                                  \
			const Vector3f v		 = entry.SP.Ray.Weight[0] * (val);                                        \
			PR_UNROLL_LOOP(3)                                                                                 \
			for (Size1i k = 0; k < 3; ++k)                                                                    \
				var_e->blendFragment(sp, k, v(k), blend);                                                     \
		}                                                                                                     \
	}

#define BLEND_3D_LPE(var, val)                                                                                \
	for (auto pair : mData.mLPE_3D[var]) {                                                                    \
		PR_OPT_LOOP                                                                                           \
		for (size_t i = 0; i < entry_count; ++i) {                                                            \
			const auto& entry		 = entries[i];                                                            \
			const LightPathView path = LightPathView(entry.Path);                                             \
			if (!pair.first.match(path))                                                                      \
				continue;                                                                                     \
			const Point2i sp		 = entry.Position + filterSize;                                           \
			const uint32 sampleCount = mData.getInternalChannel_Counter(AOV_SampleCount)->getFragment(sp, 0); \
			const float blend		 = 1.0f / (sampleCount);                                                  \
			const Vector3f v		 = entry.SP.Ray.Weight[0] * (val);                                        \
			PR_UNROLL_LOOP(3)                                                                                 \
			for (Size1i k = 0; k < 3; ++k)                                                                    \
				pair.second->blendFragment(sp, k, v(k), blend);                                               \
		}                                                                                                     \
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
	BLEND_1D(AOV_Time, entry.SP.Ray.Time);
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
	BLEND_3D(AOV_UVW, entry.SP.Surface.Geometry.UVW);
	// Check if there are any LPEs at all
	if (!mHasNonSpecLPE)
		return;

	// Handle optional LPE AOVs
	BLEND_1D_LPE(AOV_Time, entry.SP.Ray.Time);
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
	BLEND_3D_LPE(AOV_UVW, entry.SP.Surface.Geometry.UVW);
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
