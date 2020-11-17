#include "StatusObserver.h"
#include "ProgramSettings.h"
#include "Utils.h"
#include "renderer/RenderContext.h"

namespace PR {

constexpr int PERC_OUTPUT_FIELD_SIZE  = 6;	// Percentage
constexpr int ITER_OUTPUT_FIELD_SIZE  = 5;	// Current Iteration
constexpr int VALUE_OUTPUT_FIELD_SIZE = 12; // Auxiliary information
constexpr int TIME_OUTPUT_FIELD_SIZE  = 8;	// Time spent and left
#define REMOVE_LAST_LINE "\33[2K\r"
#define SAVE_CURSOR "\33[s"
#define RESTORE_CURSOR "\33[u"

StatusObserver::StatusObserver()
	: mRenderContext(nullptr)
	, mUpdateCycleSeconds(0)
	, mBeautify(true)
	, mFirstTime(true)
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
	mUpdateCycleSeconds = settings.ShowProgress;

	mLastUpdate = std::chrono::high_resolution_clock::now();
	mFirstTime	= true;
}

void StatusObserver::end()
{
	if (mBeautify)
		std::cout << REMOVE_LAST_LINE;
	std::cout << "Done" << std::setw(120) << " " << std::endl;
}

void StatusObserver::update(const UpdateInfo& info)
{
	PR_ASSERT(mRenderContext, "Invalid render context");
	const auto now			= std::chrono::high_resolution_clock::now();
	const auto duration		= std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);
	const auto fullDuration = std::chrono::duration_cast<std::chrono::seconds>(now - info.Start);

	if ((uint64)duration.count() >= mUpdateCycleSeconds) {
		RenderStatus status = mRenderContext->status();

		if (mBeautify) {
			if (mFirstTime) {
				std::cout << SAVE_CURSOR;
				mFirstTime = false;
			} else {
				std::cout << REMOVE_LAST_LINE << RESTORE_CURSOR;
			}
		}

		std::cout << std::setw(PERC_OUTPUT_FIELD_SIZE) << std::setprecision(4) << std::fixed << status.percentage() << "%";

		if (status.hasField("int.feedback"))
			std::cout << "( " << status.getField("int.feedback").getString() << ")";

		std::cout << " | I: " << std::setw(ITER_OUTPUT_FIELD_SIZE) << info.CurrentIteration
				  << " S: " << std::setw(VALUE_OUTPUT_FIELD_SIZE) << status.getField("global.pixel_sample_count").getUInt()
				  << " R: " << std::setw(VALUE_OUTPUT_FIELD_SIZE) << status.getField("global.ray_count").getUInt()
				  << " EH: " << std::setw(VALUE_OUTPUT_FIELD_SIZE) << status.getField("global.entity_hit_count").getUInt()
				  << " BH: " << std::setw(VALUE_OUTPUT_FIELD_SIZE) << status.getField("global.background_hit_count").getUInt();

		const float etaFactor = status.percentage() > PR_EPSILON ? (100 - status.percentage()) / status.percentage() : 100.0f /* Just something high*/;
		std::cout << " | RT: " << std::setw(TIME_OUTPUT_FIELD_SIZE) << timestr(fullDuration.count())
				  << " ETA: " << std::setw(TIME_OUTPUT_FIELD_SIZE) << timestr(fullDuration.count() * etaFactor);

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