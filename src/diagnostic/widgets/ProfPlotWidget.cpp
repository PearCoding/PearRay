#include "ProfPlotWidget.h"
#include "prof/ProfTreeItem.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QToolTip>

ProfPlotWidget::ProfPlotWidget(QWidget* parent)
	: QChartView(parent)
	, mAxisX(nullptr)
	, mTimeAxisY(nullptr)
	, mValueAxisY(nullptr)
	, mShowMode(0)
	, mDragging(false)
{
	setMouseTracking(true);
	chart()->setTheme(QChart::ChartThemeBlueCerulean);
	chart()->legend()->hide();

	QDateTimeAxis* axisX = new QDateTimeAxis;
	axisX->setTitleText("Time [ms]");
	axisX->setFormat("mm:ss.zzz");
	axisX->setRange(QDateTime::fromMSecsSinceEpoch(0),
					QDateTime::fromMSecsSinceEpoch(10));
	mAxisX = axisX;
	chart()->addAxis(axisX, Qt::AlignBottom); // Takes ownership

	QDateTimeAxis* timeAxisY = new QDateTimeAxis;
	timeAxisY->setTitleText("Duration [ms]");
	timeAxisY->setFormat("mm:ss.zzz");
	timeAxisY->setRange(QDateTime::fromMSecsSinceEpoch(0),
						QDateTime::fromMSecsSinceEpoch(10));
	mTimeAxisY = timeAxisY;

	QValueAxis* valAxisY = new QValueAxis;
	valAxisY->setTitleText("Calls");
	valAxisY->setRange(0, 10);
	mValueAxisY = valAxisY;

	setShowMode(0);
	setRenderHint(QPainter::Antialiasing);
}

ProfPlotWidget::~ProfPlotWidget()
{
	// One axis is not in ownership of QChart, therefore we destroy it ourselves...
	if (mShowMode == ProfTreeItem::C_TotalValue)
		delete mTimeAxisY; // Uses Value axis, destroy Time Axis
	else if (mShowMode == ProfTreeItem::C_TotalDuration
			 || mShowMode == ProfTreeItem::C_AverageDuration)
		delete mValueAxisY; // Uses Time axis, destroy Value Axis
}

void ProfPlotWidget::addTimeGraph(ProfTreeItem* item)
{
	Q_ASSERT(item);
	Q_ASSERT(!mMapper.contains(item));

	QLineSeries* series = new QLineSeries;
	series->setName(item->name());
	for (quint64 t : item->timePoints()) {
		switch (mShowMode) {
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
	fixRange(series);

	mMapper.insert(item, series);

	QPixmap icon(12, 12);
	icon.fill(series->color());
	item->setCustomIcon(icon);
}

void ProfPlotWidget::removeTimeGraph(ProfTreeItem* item)
{
	Q_ASSERT(mMapper.contains(item));

	QLineSeries* series = mMapper[item];
	chart()->removeSeries(series);
	delete series;

	item->setCustomIcon(QPixmap());
	mMapper.remove(item);
}

bool ProfPlotWidget::hasTimeGraph(ProfTreeItem* item) const
{
	return mMapper.contains(item);
}

void ProfPlotWidget::keyPressEvent(QKeyEvent* event)
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
	case Qt::Key_R:
		resetView();
		break;
	default:
		QChartView::keyPressEvent(event);
		break;
	}
}

void ProfPlotWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::MiddleButton) {
		mDragging = true;
		mLastPos  = event->screenPos();
		QApplication::setOverrideCursor(Qt::ClosedHandCursor);
		event->accept();
	} else {
		QChartView::mousePressEvent(event);
	}
}

void ProfPlotWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (mDragging && !(event->buttons() & Qt::MiddleButton)) {
		mDragging = false;
		QApplication::restoreOverrideCursor();
		event->accept();
	} else {
		QChartView::mouseReleaseEvent(event);
	}
}

void ProfPlotWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (mDragging && (event->buttons() & Qt::MiddleButton)) {
		auto delta = event->screenPos() - mLastPos;
		mLastPos   = event->screenPos();
		chart()->scroll(-delta.x(), delta.y());
		event->accept();
	} else {
		QChartView::mouseMoveEvent(event);
	}

	// Show tooltip
	QPointF pos = chart()->mapToScene(event->localPos());
	if (!chart()->plotArea().contains(pos) || chart()->series().isEmpty())
		return;

	QPointF val = chart()->mapToValue(pos);
	QToolTip::showText(
		event->globalPos(),
		QString("[%1, %2]").arg(val.x()).arg(val.y()),
		this,
		rect());
}

void ProfPlotWidget::wheelEvent(QWheelEvent* event)
{
	if (chart()->series().isEmpty())
		return;

	constexpr float ZOOM_FACTOR = 1.5f;
	if (event->delta() < 0)
		chart()->zoom(1 / ZOOM_FACTOR);
	else if (event->delta() > 0)
		chart()->zoom(ZOOM_FACTOR);
}

void ProfPlotWidget::setShowMode(int mode)
{
	// Remove previous axis
	if (mShowMode == ProfTreeItem::C_TotalValue)
		chart()->removeAxis(mValueAxisY);
	else if (mShowMode == ProfTreeItem::C_TotalDuration
			 || mShowMode == ProfTreeItem::C_AverageDuration)
		chart()->removeAxis(mTimeAxisY);

	mShowMode = mode + ProfTreeItem::C_TotalValue; // C_Name is not valid

	switch (mShowMode) {
	case ProfTreeItem::C_TotalValue:
		chart()->setTitle("Total Calls");
		break;
	case ProfTreeItem::C_TotalDuration:
		chart()->setTitle("Total Duration");
		break;
	case ProfTreeItem::C_AverageDuration:
		chart()->setTitle("Average Duration");
		break;
	}

	if (mShowMode == ProfTreeItem::C_TotalValue)
		chart()->addAxis(mValueAxisY, Qt::AlignLeft);
	else
		chart()->addAxis(mTimeAxisY, Qt::AlignLeft);

	QList<ProfTreeItem*> keys = mMapper.keys();

	mMapper.clear();
	chart()->removeAllSeries();

	for (ProfTreeItem* key : keys)
		addTimeGraph(key);

	resetView();
}

void ProfPlotWidget::fixRange(QLineSeries* series)
{
	Q_ASSERT(series);

	qreal maxX = 0, maxY = 0;
	for (const auto& p : series->points()) {
		maxX = std::max(maxX, p.x());
		maxY = std::max(maxY, p.y());
	}

	series->attachAxis(mAxisX);
	if (reinterpret_cast<QDateTimeAxis*>(mAxisX)->max()
		< QDateTime::fromMSecsSinceEpoch(maxX))
		mAxisX->setMax(QDateTime::fromMSecsSinceEpoch(maxX));

	if (mShowMode == ProfTreeItem::C_TotalValue) {
		series->attachAxis(mValueAxisY);
		if (reinterpret_cast<QValueAxis*>(mValueAxisY)->max() < maxY)
			mValueAxisY->setMax(maxY);
	} else {
		series->attachAxis(mTimeAxisY);
		if (reinterpret_cast<QDateTimeAxis*>(mTimeAxisY)->max()
			< QDateTime::fromMSecsSinceEpoch(maxY))
			mTimeAxisY->setMax(QDateTime::fromMSecsSinceEpoch(maxY));
	}
}

void ProfPlotWidget::resetView()
{
	chart()->zoomReset();
}