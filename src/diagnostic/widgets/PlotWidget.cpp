#include "PlotWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <QMenu>

constexpr float PAN_W  = 0.4f;
constexpr float ZOOM_W = 1.05f;

constexpr int GRID_S = 100;

PlotWidget::PlotWidget(QWidget* parent)
	: QWidget(parent)
	, mZoom(1)
{
	setMouseTracking(true);
}

PlotWidget::~PlotWidget()
{
}

QSize PlotWidget::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize PlotWidget::sizeHint() const
{
	return minimumSizeHint();
}

void PlotWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, mBackground);

	event->accept();
}

// Cache background
void PlotWidget::resizeEvent(QResizeEvent* event)
{
	QPainter painter;
	mBackground = QPixmap(event->size());

	painter.begin(&mBackground);

	// Background
	const size_t gx = mBackground.width() / GRID_S + 1;
	const size_t gy = mBackground.height() / GRID_S + 1;

	painter.setPen(Qt::NoPen);
	for (size_t y = 0; y < gy; ++y) {
		int dx = (y % 2) ? 0 : GRID_S;
		int py = y * GRID_S;

		painter.setBrush(QBrush(Qt::white));
		for (size_t x = 0; x < gx; ++x)
			painter.drawRect(dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);

		painter.setBrush(QBrush(qRgb(240,240,240)));
		for (size_t x = 0; x < gx; ++x)
			painter.drawRect(GRID_S - dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);
	}
	painter.end();
}

void PlotWidget::mousePressEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		mLastPos = event->globalPos();
	}
}

void PlotWidget::mouseMoveEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		QPointF d = mLastPos - event->globalPos();

		mDelta += -d * PAN_W;
		mLastPos = event->globalPos();
	}

	repaint();
}

void PlotWidget::wheelEvent(QWheelEvent* event)
{
	float delta = event->angleDelta().y();

	if (delta < 0)
		mZoom /= ZOOM_W;
	else if (delta > 0)
		mZoom *= ZOOM_W;

	event->accept();

	cachePlot();
	repaint();
}

void PlotWidget::cachePlot()
{
	// TODO
}

void PlotWidget::updatePlot()
{
	// TODO
	cachePlot();
	repaint();
}
