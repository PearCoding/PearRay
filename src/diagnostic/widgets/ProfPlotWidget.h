#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QSignalMapper>

QT_CHARTS_USE_NAMESPACE

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

	QAbstractAxis* mAxisX;
	QAbstractAxis* mTimeAxisY;
	QAbstractAxis* mValueAxisY;

	int mShowMode;
	QHash<ProfTreeItem*, QLineSeries*> mMapper;

	QPointF mLastPos;
	bool mDragging;
};