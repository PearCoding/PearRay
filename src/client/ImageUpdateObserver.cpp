#include "ImageUpdateObserver.h"
#include "Environment.h"
#include "ProgramSettings.h"
#include "renderer/RenderContext.h"

namespace PR {
ImageUpdateObserver::ImageUpdateObserver(Environment* environment)
	: mRenderContext(nullptr)
	, mFrameOutputDevice(nullptr)
	, mEnvironment(environment)
	, mIterationCount(0)
	, mIterationCycleCount(0)
	, mUpdateCycleSeconds(0)
	, mUseTags(false)
{
	PR_ASSERT(mEnvironment, "Invalid environment");
}

ImageUpdateObserver::~ImageUpdateObserver()
{
}

void ImageUpdateObserver::begin(RenderContext* renderContext, FrameOutputDevice* outputDevice, const ProgramSettings& settings)
{
	PR_ASSERT(renderContext, "Invalid render context");
	mRenderContext		 = renderContext;
	mFrameOutputDevice	 = outputDevice;
	mUseTags			 = settings.ImgUseTags;
	mIterationCount		 = 0;
	mIterationCycleCount = settings.ImgUpdateIteration;
	mUpdateCycleSeconds	 = settings.ImgUpdate;

	mLastUpdate = std::chrono::high_resolution_clock::now();
}

void ImageUpdateObserver::end()
{
	// Nothing
}

void ImageUpdateObserver::update(const UpdateInfo& info)
{
	if (info.CurrentPass != 0)
		return;

	auto now	  = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);

	if (mUpdateCycleSeconds > 0 && (uint64)duration.count() >= mUpdateCycleSeconds) {
		save(info);
		mLastUpdate = now;
	}
}

void ImageUpdateObserver::onIteration(const UpdateInfo& info)
{
	if (info.CurrentPass != 0)
		return;

	if (mIterationCycleCount > 0 && mIterationCount >= mIterationCycleCount) {
		save(info);
		mIterationCount = 0;
	}
}

void ImageUpdateObserver::save(const UpdateInfo& info)
{
	PR_ASSERT(mRenderContext, "Invalid render context");
	PR_ASSERT(mEnvironment, "Invalid environment");

	OutputSaveOptions output_options;

	output_options.Image.IterationMeta = info.CurrentIteration;
	output_options.Image.TimeMeta	   = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - info.Start).count();
	output_options.Image.WriteMeta	   = true;

	if (mUseTags) {
		std::stringstream stream;
		stream << "_i" << output_options.Image.IterationMeta << "_t" << output_options.Image.TimeMeta;
		output_options.NameSuffix = stream.str();
	}

	mEnvironment->save(mRenderContext, mFrameOutputDevice, mToneMapper, output_options);
}

} // namespace PR