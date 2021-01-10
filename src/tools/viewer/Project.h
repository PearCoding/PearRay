#pragma once

#include "renderer/RenderStatus.h"

#include <QObject>
#include <QRect>
#include <QTimer>
#include <QVector>

namespace PR {
class Environment;
class FrameOutputDevice;
class RenderContext;
} // namespace PR

class Project : public QObject {
	Q_OBJECT
public:
	explicit Project(const QString& file) noexcept(false);
	~Project();

	bool isRendering() const;
	inline bool isProgressive() const { return mProgressive; }

	inline std::shared_ptr<PR::FrameOutputDevice> frame() const { return mFrame; }
	inline const QVector<QRect>& currentUpdateRegions() const { return mUpdateRegions; }

	inline std::chrono::milliseconds renderTime() const { return std::chrono::duration_cast<std::chrono::milliseconds>(mEndTime - mStartTime); }
	std::chrono::milliseconds renderEta() const;
	PR::RenderStatus renderStatus() const;
	uint32_t renderIteration() const;

public slots:
	void startRendering(int iterations = 0, int threads = 0);
	void stopRendering(bool hard = false);

signals:
	void renderingStarted();
	void renderingFinished();

private slots:
	void checkContext();

private:
	QTimer mCheckTimer;

	std::shared_ptr<PR::Environment> mEnvironment;
	std::shared_ptr<PR::RenderContext> mContext;
	std::shared_ptr<PR::FrameOutputDevice> mFrame;

	QVector<QRect> mUpdateRegions;
	bool mProgressive;

	std::chrono::high_resolution_clock::time_point mStartTime;
	std::chrono::high_resolution_clock::time_point mEndTime;
};