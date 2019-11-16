#pragma once

#include <QWidget>
#include <memory>

class ImageBufferView;
class PlotWidget : public QWidget {
	Q_OBJECT
public:
	PlotWidget(QWidget* parent = nullptr);
	virtual ~PlotWidget();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	void updatePlot();
	void cachePlot();

	float mZoom;
	QPointF mDelta;
	QPoint mLastPos;

	QPixmap mPixmap;
	QPixmap mBackground;
};