#include "ImageUpdateObserver.h"
#include "Environment.h"
#include "ProgramSettings.h"
#include "renderer/RenderContext.h"

namespace PR {
ImageUpdateObserver::ImageUpdateObserver(Environment* environment)
	: mRenderContext(nullptr)
	, mEnvironment(environment)
	, mIterationCount(0)
	, mIterationCycleCount(0)
	, mUpdateCycleSeconds(0)
	, mUseTags(false)
	, mMaxIterationCount(0)
{
}

ImageUpdateObserver::~ImageUpdateObserver()
{
}

void ImageUpdateObserver::begin(RenderContext* renderContext, const ProgramSettings& settings)
{
	mRenderContext		 = renderContext;
	mUseTags			 = settings.ImgUseTags;
	mIterationCount		 = 0;
	mIterationCycleCount = settings.ImgUpdateIteration;
	mUpdateCycleSeconds	 = settings.ImgUpdate;
	mMaxIterationCount	 = renderContext->maxIterationCount();

	mLastUpdate = std::chrono::high_resolution_clock::now();
}

void ImageUpdateObserver::end()
{
	// Nothing
}

void ImageUpdateObserver::update(const UpdateInfo& info)
{
	auto now	  = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);

	if (mUpdateCycleSeconds > 0 && (uint64)duration.count() >= mUpdateCycleSeconds) {
		save(info);
		mLastUpdate = now;
	}
}

void ImageUpdateObserver::onIteration(const UpdateInfo& info)
{
	if (info.CurrentIteration < mMaxIterationCount && mIterationCycleCount > 0 && mIterationCount >= mIterationCycleCount) {
		save(info);
		mIterationCount = 0;
	}
}

void ImageUpdateObserver::save(const UpdateInfo& info)
{
	OutputSaveOptions output_options;

	output_options.Image.IterationMeta	= info.CurrentIteration;
	output_options.Image.TimeMeta		= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - info.Start).count();
	output_options.Image.WriteMeta		= true;
	output_options.Image.SpectralFactor = std::max(1.0f, mMaxIterationCount - info.CurrentIteration - 1.0f);

	if (mUseTags) {
		std::stringstream stream;
		stream << "_i" << output_options.Image.IterationMeta << "_t" << output_options.Image.TimeMeta;
		output_options.NameSuffix = stream.str();
	}

	mEnvironment->save(mRenderContext, mToneMapper, output_options);
}

} // namespace PR