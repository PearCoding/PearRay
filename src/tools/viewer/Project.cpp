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
{
	mCheckTimer.setTimerType(Qt::CoarseTimer);

	QFileInfo info(file);

	SceneLoader::LoadOptions opts;
	opts.WorkingDir	 = info.absoluteDir().path().toLocal8Bit().constData();
	opts.Progressive = true;

	mEnvironment = SceneLoader::loadFromFile(file.toLocal8Bit().constData(), opts);
	mFactory	 = mEnvironment->createRenderFactory();

	connect(&mCheckTimer, &QTimer::timeout, this, &Project::checkContext);
}

Project::~Project()
{
}

bool Project::isRendering() const
{
	return mContext && !mContext->isFinished();
}

void Project::startRendering(int threads)
{
	if (mContext && !mContext->isFinished())
		return;

	auto integrator = mEnvironment->createSelectedIntegrator();
	mContext		= mFactory->create(integrator);
	mFrame			= mEnvironment->createAndAssignFrameOutputDevice(mContext);

	mContext->start(64, 64, threads);
	mCheckTimer.start(TIMER_MSECS);

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

	if (mContext->isFinished()) {
		mCheckTimer.stop();
		emit renderingFinished();
		return;
	}
}