#include "StatusObserver.h"
#include "ProgramSettings.h"
#include "Utils.h"
#include "renderer/RenderContext.h"

namespace PR {
StatusObserver::StatusObserver()
	: mRenderContext(nullptr)
	, mUpdateCycleSeconds(0)
	, mBeautify(true)
{
}

StatusObserver::~StatusObserver()
{
}

void StatusObserver::begin(RenderContext* renderContext, const ProgramSettings& settings)
{
	PR_ASSERT(renderContext, "Invalid render context");
	mRenderContext		= renderContext;
	mBeautify			= !settings.NoPrettyConsole;
	mUpdateCycleSeconds = settings.ImgUpdate;

	mLastUpdate = std::chrono::high_resolution_clock::now();

	std::cout << "preprocess" << std::endl; // TODO: Only for the Blender plugin -> Bad code
}

constexpr int OUTPUT_FIELD_SIZE = 8;
#define REMOVE_LAST_LINE "\33[2K\r"

void StatusObserver::end()
{
	if (mBeautify)
		std::cout << REMOVE_LAST_LINE;
	std::cout << "Done" << std::setw(120) << " " << std::endl;
}

void StatusObserver::update(const UpdateInfo&)
{
	PR_ASSERT(mRenderContext, "Invalid render context");
	const auto now		= std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);

	if ((uint64)duration.count() >= mUpdateCycleSeconds) {
		RenderStatus status = mRenderContext->status();

		if (mBeautify)
			std::cout << REMOVE_LAST_LINE;

		std::cout << std::setw(OUTPUT_FIELD_SIZE) << std::setprecision(4) << std::fixed
				  << status.percentage() * 100 << "%";

		if (status.hasField("int.feedback"))
			std::cout << "( " << status.getField("int.feedback").getString() << ")";

		std::cout << " | S: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.pixel_sample_count").getUInt()
				  << " R: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.ray_count").getUInt()
				  << " EH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.entity_hit_count").getUInt()
				  << " BH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.background_hit_count").getUInt();

		std::cout << " | RT: " << std::setw(OUTPUT_FIELD_SIZE) << timestr(duration.count())
				  << " ETA: " << std::setw(OUTPUT_FIELD_SIZE) << timestr(duration.count() * ((1 - status.percentage()) / status.percentage()));

		if (!mBeautify)
			std::cout << std::endl;
		else
			std::cout << std::flush;

		mLastUpdate = now;
	}
}

void StatusObserver::onIteration(const UpdateInfo&)
{
	// Nothing
}

} // namespace PR