#include "Project.h"

#include "Environment.h"
#include "SceneLoader.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"

#include <QDir>
#include <QFileInfo>

using namespace PR;

// Interval to check for the rendering context
constexpr int TIMER_MSECS = 100;

Project::Project(const QString& file) noexcept(false)
	: mCheckTimer(this)
	, mProgressive(false)
	, mStartTime()
	, mEndTime()
{
	mCheckTimer.setTimerType(Qt::CoarseTimer);

	QFileInfo info(file);

	SceneLoader::LoadOptions opts;
	opts.WorkingDir	 = info.absoluteDir().path().toLocal8Bit().constData();
	opts.Progressive = true; // Regardless if actually progressive or not, the internal should be loaded with progressive in mind

	mEnvironment = SceneLoader::loadFromFile(file.toLocal8Bit().constData(), opts);

	connect(&mCheckTimer, &QTimer::timeout, this, &Project::checkContext);
}

Project::~Project()
{
}

bool Project::isRendering() const
{
	return mContext && !mContext->isFinished();
}

void Project::startRendering(int iterations, int threads)
{
	if (mContext && !mContext->isFinished())
		return;

	mProgressive									   = iterations <= 0;
	mEnvironment->renderSettings().progressive		   = mProgressive;
	mEnvironment->renderSettings().sampleCountOverride = std::max(0, iterations);

	auto factory	= mEnvironment->createRenderFactory();
	auto integrator = mEnvironment->createSelectedIntegrator();
	mContext		= factory->create(integrator);
	mFrame			= mEnvironment->createAndAssignFrameOutputDevice(mContext);

	mContext->start(64, 64, threads);
	mCheckTimer.start(TIMER_MSECS);
	mStartTime = std::chrono::high_resolution_clock::now();
	mEndTime   = mStartTime;

	emit renderingStarted();
}

void Project::stopRendering(bool hard)
{
	if (mContext) {
		if (hard)
			mContext->requestStop();
		else
			mContext->requestSoftStop();
	}
}

void Project::checkContext()
{
	if (!mContext) {
		mCheckTimer.stop();
		return;
	}

	mEndTime = std::chrono::high_resolution_clock::now();

	if (mContext->isFinished()) {
		mCheckTimer.stop();
		emit renderingFinished();
		return;
	}

	mUpdateRegions.clear();
	const auto regions = mContext->currentTiles();
	if (!regions.empty()) {
		mUpdateRegions.reserve(regions.size());
		for (const auto& r : regions)
			mUpdateRegions.append(QRect(QPoint(r.Origin(0), r.Origin(1)), QSize(r.Size.Width, r.Size.Height)));
	}
}

PR::RenderStatus Project::renderStatus() const
{
	if (!isRendering())
		return PR::RenderStatus();

	return mContext->status();
}

std::chrono::milliseconds Project::renderEta() const
{
	if (!isRendering())
		return std::chrono::milliseconds(0);

	if (mContext->settings().progressive)
		return std::chrono::milliseconds(0);

	const auto status	  = renderStatus();
	const float etaFactor = status.percentage() > PR_EPSILON ? (100 - status.percentage()) / status.percentage() : 100.0f /* Just something high*/;
	return std::chrono::milliseconds(static_cast<int64_t>(renderTime().count() * etaFactor));
}

uint32_t Project::renderIteration() const
{
	if (!isRendering())
		return 0;

	return mContext->currentIteration().Iteration;
}