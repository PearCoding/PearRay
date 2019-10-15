#include "RangeSlider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <cmath>

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
	, mPickDelta(0)
{
}

RangeSlider::~RangeSlider()
{
}

constexpr size_t BAR_H	= 20;
constexpr size_t KNOB_W   = 10;
constexpr size_t BORDER_W = 5;
constexpr size_t MIN_W	= BORDER_W * 2 + KNOB_W * 2;

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

	return sty;
}

QSize RangeSlider::minimumSizeHint() const
{
	return QSize(MIN_W, BAR_H);
}

QSize RangeSlider::sizeHint() const
{
	size_t w = std::ceil(mMax - mMin);
	return QSize(MIN_W + w, BAR_H);
}

void RangeSlider::setLeft(float f)
{
	float old = mLeft;
	mLeft	 = qMax(mMin, qMin(mRight, f));
	if (old != mLeft)
		emit leftChanged(mLeft);
}

void RangeSlider::setRight(float f)
{
	float old = mRight;
	mRight	= qMin(mMax, qMax(mLeft, f));
	if (old != mRight)
		emit rightChanged(mRight);
}

void RangeSlider::setMin(float f)
{
	mMin = f;
	if (mMin > mMax)
		qSwap(mMin, mMax);

	setLeft(mLeft);
	setRight(mRight);
}

void RangeSlider::setMax(float f)
{
	mMax = f;
	if (mMin > mMax)
		qSwap(mMin, mMax);

	setLeft(mLeft);
	setRight(mRight);
}

void RangeSlider::fitFull()
{
	setLeft(mMin);
	setRight(mMax);
}

constexpr QRgb KNOB_C  = qRgb(78, 167, 245);
constexpr QRgb SLIDE_B = qRgb(52, 149, 235);
constexpr QRgb SLIDE_P = qRgb(27, 91, 148);

void RangeSlider::paintEvent(QPaintEvent* event)
{
	size_t h	  = height();
	DrawStyle sty = calculateStyle();

	QPainter painter(this);

	// Background
	painter.setBrush(QBrush(Qt::gray));
	painter.drawRect(QRect(0, 0, width() - 1, h - 1));

	// Border
	painter.setBrush(QBrush(Qt::darkGray));
	painter.drawRect(QRect(0, 0, BORDER_W - 1, h - 1));
	painter.drawRect(QRect(width() - BORDER_W - 1, 0, BORDER_W - 1, h - 1));

	painter.setPen(QPen(SLIDE_P));
	// Knob Left
	painter.setBrush(QBrush(KNOB_C));
	painter.drawRect(QRect(sty.LeftKnobStart, 1, KNOB_W - 1, h - 3));

	// Knob Right
	painter.drawRect(QRect(sty.RightKnobStart, 1, KNOB_W - 1, h - 3));

	// Slide
	painter.setBrush(QBrush(SLIDE_B));
	painter.drawRect(QRect(sty.SlideStart, 1,
						   sty.SlideEnd - sty.SlideStart, h - 3));

	painter.setBrush(QBrush(Qt::darkGray));
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
	DrawStyle sty	= calculateStyle();

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
			setLeft(posToValue(event->x()));
			repaint();
			break;
		case MO_KNOB_RIGHT:
			setRight(posToValue(event->x()));
			repaint();
			break;
		case MO_PAN: {
			int dx   = event->globalX() - mLastX;
			float df = dx / (float)wcw;

			if (dx < 0) {
				float ddf = qMin(0.0f, (mLeft - mMin) + df);
				setLeft(mLeft + df);
				setRight(mRight + df - ddf);
				repaint();
			} else if (dx > 0) {
				float ddf = qMin(0.0f, (mMax - mRight) - df);
				setRight(mRight + df);
				setLeft(mLeft + df + ddf);
				repaint();
			}

			mLastX = event->globalX();
		} break;
		default:
			break;
		}
	}
}

void RangeSlider::wheelEvent(QWheelEvent* event)
{
}
