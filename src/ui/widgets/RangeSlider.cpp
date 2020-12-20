#include "RangeSlider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#ifndef QT_NO_CONTEXTMENU
#include <QMenu>
#endif

#include <cmath>

namespace PR {
namespace UI {
RangeSlider::RangeSlider(QWidget* parent)
	: RangeSlider(0, 1, parent)
{
}

RangeSlider::RangeSlider(float min, float max, QWidget* parent)
	: QWidget(parent)
	, mLeft(min)
	, mRight(max)
	, mMin(min)
	, mMax(max)
	, mCurrentOperation(MO_NONE)
	, mLastX(0)
	, mPickDelta(0)
{
}

RangeSlider::~RangeSlider()
{
}

constexpr int BAR_H	   = 20;
constexpr int KNOB_W   = 10;
constexpr int BORDER_W = 5;
constexpr int MIN_W	   = BORDER_W * 2 + KNOB_W * 2;

constexpr QRgb KNOB_COLOR				= qRgb(78, 167, 245);
constexpr QRgb KNOB_COLOR_INACTIVE		= qRgb(169, 204, 235);
constexpr QRgb SLIDE_BAR_COLOR			= qRgb(52, 149, 235);
constexpr QRgb SLIDE_BAR_COLOR_INACTIVE = qRgb(121, 168, 209);
constexpr QRgb SLIDE_P					= qRgb(27, 91, 148);

RangeSlider::DrawStyle RangeSlider::calculateStyle() const
{
	DrawStyle sty;

	size_t cw  = std::ceil(mMax - mMin);
	float fs   = (mLeft - mMin) / (float)cw;
	float fe   = (mRight - mMin) / (float)cw;
	size_t wcw = width() - MIN_W;

	sty.SlideStart = BORDER_W + KNOB_W + fs * wcw;
	sty.SlideEnd   = BORDER_W + KNOB_W + fe * wcw;

	sty.LeftKnobStart  = sty.SlideStart - KNOB_W;
	sty.RightKnobStart = sty.SlideEnd + 1;

	sty.SlideBarColor  = isEnabled() ? SLIDE_BAR_COLOR : SLIDE_BAR_COLOR_INACTIVE;
	sty.SlideKnobColor = isEnabled() ? KNOB_COLOR : KNOB_COLOR_INACTIVE;
	sty.TextColor	   = isEnabled() ? Qt::black : Qt::gray;

	return sty;
}

QSize RangeSlider::minimumSizeHint() const
{
	return QSize(MIN_W, BAR_H);
}

QSize RangeSlider::sizeHint() const
{
	int w = std::ceil(mMax - mMin);
	return QSize(MIN_W + w, BAR_H);
}

void RangeSlider::setLeftValue(float f)
{
	float old = mLeft;
	mLeft	  = qMax(mMin, qMin(mRight, f));
	if (old != mLeft)
		emit leftValueChanged(mLeft);
}

void RangeSlider::setRightValue(float f)
{
	float old = mRight;
	mRight	  = qMin(mMax, qMax(mLeft, f));
	if (old != mRight)
		emit rightValueChanged(mRight);
}

void RangeSlider::setMinValue(float f)
{
	mMin = f;
	if (mMin > mMax)
		qSwap(mMin, mMax);

	setLeftValue(mLeft);
	setRightValue(mRight);
}

void RangeSlider::setMaxValue(float f)
{
	mMax = f;
	if (mMin > mMax)
		qSwap(mMin, mMax);

	setLeftValue(mLeft);
	setRightValue(mRight);
}

void RangeSlider::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	int h				= height();
	const QRect content = QRect(0, 0, width() - 1, h - 1);
	const QString text	= QString("%1 | %2").arg(mLeft, 0, 'g', 3).arg(mRight, 0, 'g', 3);
	DrawStyle sty		= calculateStyle();

	QPainter painter(this);

	// Background
	painter.setBrush(QBrush(Qt::gray));
	painter.drawRect(content);

	// Border
	painter.setBrush(QBrush(Qt::darkGray));
	painter.drawRect(QRect(0, 0, BORDER_W - 1, h - 1));
	painter.drawRect(QRect(width() - BORDER_W - 1, 0, BORDER_W - 1, h - 1));

	painter.setPen(QPen(SLIDE_P));
	// Knob Left
	painter.setBrush(QBrush(sty.SlideKnobColor));
	painter.drawRect(QRect(sty.LeftKnobStart, 1, KNOB_W - 1, h - 3));

	// Knob Right
	painter.drawRect(QRect(sty.RightKnobStart, 1, KNOB_W - 1, h - 3));

	// Slide
	painter.setBrush(QBrush(sty.SlideBarColor));
	painter.drawRect(QRect(sty.SlideStart, 1,
						   sty.SlideEnd - sty.SlideStart, h - 3));

	// Text
	painter.setPen(QPen(sty.TextColor));
	painter.drawText(content, text, QTextOption(Qt::AlignCenter));
}

void RangeSlider::mousePressEvent(QMouseEvent* event)
{
	DrawStyle sty = calculateStyle();
	if (event->buttons() & Qt::LeftButton) {
		int x = event->x();
		if (sty.LeftKnobStart <= x && x <= sty.LeftKnobStart + KNOB_W) {
			mCurrentOperation = MO_KNOB_LEFT;
			mPickDelta		  = x - (int)sty.LeftKnobStart;
		} else if (sty.RightKnobStart <= x && x <= sty.RightKnobStart + KNOB_W) {
			mCurrentOperation = MO_KNOB_RIGHT;
			mPickDelta		  = (int)sty.RightKnobStart - x;
		} else if (sty.SlideStart <= x && x <= sty.SlideEnd)
			mCurrentOperation = MO_PAN;
		else
			mCurrentOperation = MO_NONE;

		mLastX = event->globalX();
	}
}

void RangeSlider::mouseMoveEvent(QMouseEvent* event)
{
	const size_t wcw = width() - MIN_W;

	auto posToValue = [&](int x) {
		return qMin(1.0f, qMax(0.0f,
							   (x - (BORDER_W + KNOB_W - mPickDelta))
								   / (float)wcw))
				   * std::ceil(mMax - mMin)
			   + mMin;
	};

	if (event->buttons() & Qt::LeftButton) {
		switch (mCurrentOperation) {
		case MO_KNOB_LEFT:
			setLeftValue(posToValue(event->x()));
			repaint();
			break;
		case MO_KNOB_RIGHT:
			setRightValue(posToValue(event->x()));
			repaint();
			break;
		case MO_PAN: {
			int dx = event->globalX() - mLastX;
			pan(dx);
			mLastX = event->globalX();
		} break;
		default:
			break;
		}
	}
}

void RangeSlider::wheelEvent(QWheelEvent* event)
{
	pan(event->delta() / 15);
}

void RangeSlider::pan(int dx)
{
	float df = dx / (float)(width() - MIN_W);

	if (dx < 0) {
		float ddf = qMin(0.0f, (mLeft - mMin) + df);
		setLeftValue(mLeft + df);
		setRightValue(mRight + df - ddf);
		repaint();
	} else if (dx > 0) {
		float ddf = qMin(0.0f, (mMax - mRight) - df);
		setRightValue(mRight + df);
		setLeftValue(mLeft + df + ddf);
		repaint();
	}
}

#ifndef QT_NO_CONTEXTMENU
void RangeSlider::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);
	menu.addAction("Normal", [this]() { this->setLeftValue(0); this->setRightValue(1); this->update(); });
	menu.addAction("Fill", [this]() { this->setLeftValue(-PR_INF); this->setRightValue(PR_INF); this->update(); });
	menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU
} // namespace UI
} // namespace PR