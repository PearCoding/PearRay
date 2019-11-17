#include "PlotWidget.h"
#include "prof/ProfTreeItem.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#include <QDateTime>
#include <QDebug>

PlotWidget::PlotWidget(QWidget* parent)
	: QChartView(parent)
{
	setRubberBand(QChartView::RectangleRubberBand);
	chart()->legend()->hide();

	QDateTimeAxis* axisX = new QDateTimeAxis;
	axisX->setTitleText("Time");
	axisX->setFormat("HH:mm:ss.zzz");
	//axisX->setTickCount(10);
	chart()->addAxis(axisX, Qt::AlignBottom);
	mAxisX = axisX;

	QDateTimeAxis* timeAxisY = new QDateTimeAxis;
	timeAxisY->setTitleText("Duration");
	timeAxisY->setFormat("HH:mm:ss.zzz");
	//timeAxisY->setTickCount(10);
	chart()->addAxis(timeAxisY, Qt::AlignLeft);
	mTimeAxisY = timeAxisY;

	QValueAxis* valAxisY = new QValueAxis;
	valAxisY->setTitleText("Calls");
	chart()->addAxis(valAxisY, Qt::AlignLeft);
	mValueAxisY = valAxisY;

	setRenderHint(QPainter::Antialiasing);
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::addTimeGraph(ProfTreeItem* item)
{
	int currentType = ProfTreeItem::C_TotalDuration;

	QLineSeries* series = new QLineSeries;
	series->setName(item->name());
	for (quint64 t : item->timePoints()) {
		switch (currentType) {
		case ProfTreeItem::C_TotalValue:
			series->append(t / 1000, item->totalValue(t));
			break;
		case ProfTreeItem::C_TotalDuration:
			series->append(t / 1000, item->totalDuration(t) / 1000);
			break;
		case ProfTreeItem::C_AverageDuration:
			series->append(t / 1000, item->totalDuration(t) / (qreal)(1000 * item->totalValue(t)));
			break;
		default:
			return;
		}
	}

	chart()->addSeries(series); // Takes ownership!

	series->attachAxis(mAxisX);
	if (currentType == ProfTreeItem::C_TotalValue)
		series->attachAxis(mValueAxisY);
	else
		series->attachAxis(mTimeAxisY);

	mMapper.insert(item, series);

	QPixmap icon(8, 8);
	icon.fill(series->color());
	item->setCustomIcon(icon);
}

void PlotWidget::removeTimeGraph(ProfTreeItem* item)
{
	Q_ASSERT(mMapper.contains(item));

	QLineSeries* series = mMapper[item];
	chart()->removeSeries(series);
	delete series;

	item->setCustomIcon(QPixmap());
	mMapper.remove(item);
}

bool PlotWidget::hasTimeGraph(ProfTreeItem* item) const
{
	return mMapper.contains(item);
}

void PlotWidget::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Plus:
		chart()->zoomIn();
		break;
	case Qt::Key_Minus:
		chart()->zoomOut();
		break;
	case Qt::Key_Left:
		chart()->scroll(-10, 0);
		break;
	case Qt::Key_Right:
		chart()->scroll(10, 0);
		break;
	case Qt::Key_Up:
		chart()->scroll(0, 10);
		break;
	case Qt::Key_Down:
		chart()->scroll(0, -10);
		break;
	default:
		QGraphicsView::keyPressEvent(event);
		break;
	}
}