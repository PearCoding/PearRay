#pragma once

#include "PR_Config.h"

#include <QObject>
#include <QTimer>
#include <QRect>

namespace PR {
class Environment;
class FrameOutputDevice;
class RenderFactory;
class RenderContext;
} // namespace PR

class Project : public QObject {
	Q_OBJECT
public:
	explicit Project(const QString& file) noexcept(false);
	~Project();

	bool isRendering() const;

	inline std::shared_ptr<PR::FrameOutputDevice> frame() const { return mFrame; }
	inline const QVector<QRect>& currentUpdateRegions() const { return mUpdateRegions; }

public slots:
	void startRendering(int threads = 0);
	void stopRendering(bool hard = false);

signals:
	void renderingStarted();
	void renderingFinished();

private slots:
	void checkContext();

private:
	QTimer mCheckTimer;

	std::shared_ptr<PR::Environment> mEnvironment;
	std::shared_ptr<PR::RenderFactory> mFactory;
	std::shared_ptr<PR::RenderContext> mContext;
	std::shared_ptr<PR::FrameOutputDevice> mFrame;

	QVector<QRect> mUpdateRegions;
};