#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class ProfTreeItem;
class PlotWidget : public QChartView {
	Q_OBJECT
public:
	PlotWidget(QWidget* parent = nullptr);
	virtual ~PlotWidget();

	void addTimeGraph(ProfTreeItem* root);
	void removeTimeGraph(ProfTreeItem* item);
	bool hasTimeGraph(ProfTreeItem* item) const;

protected:
	void keyPressEvent(QKeyEvent* event);

private:
	QAbstractAxis* mAxisX;
	QAbstractAxis* mTimeAxisY;
	QAbstractAxis* mValueAxisY;

	QHash<ProfTreeItem*, QLineSeries*> mMapper;
};