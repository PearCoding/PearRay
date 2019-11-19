#pragma once

#include <QSignalMapper>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

enum ProfShowMode {
	PSM_TotalCalls = 0,
	PSM_DeltaCalls,
	PSM_TotalDuration,
	PSM_DeltaDuration,
	PSM_AverageDuration
};

class ProfTreeItem;
class ProfPlotWidget : public QChartView {
	Q_OBJECT
public:
	ProfPlotWidget(QWidget* parent = nullptr);
	virtual ~ProfPlotWidget();

	void addTimeGraph(ProfTreeItem* item);
	void removeTimeGraph(ProfTreeItem* item);
	bool hasTimeGraph(ProfTreeItem* item) const;

public slots:
	void setShowMode(int mode);
	void resetView();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	void fixRange(QLineSeries* series);
	void setupCurrentMode();
	inline bool isUsingValueAxis() const
	{
		return mShowMode == PSM_TotalCalls || mShowMode == PSM_DeltaCalls;
	}

	QAbstractAxis* mAxisX;
	QAbstractAxis* mTimeAxisY;
	QAbstractAxis* mValueAxisY;

	ProfShowMode mShowMode;
	QHash<ProfTreeItem*, QLineSeries*> mMapper;

	QPointF mLastPos;
	bool mDragging;
};