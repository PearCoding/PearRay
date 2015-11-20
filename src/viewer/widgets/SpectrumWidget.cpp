#include "SpectrumWidget.h"
#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include <QPainter>
#include <QMouseEvent>

SpectrumWidget::SpectrumWidget(QWidget *parent)
	: QWidget(parent),
	mSpectrum(), mCurrentNM(-1),
	mSpecMax(1)
{
	cache();

	setMouseTracking(true);
}

SpectrumWidget::~SpectrumWidget()
{
}

constexpr int PADDING = 5;
constexpr int SAMPLE_SPACING = 5;
constexpr int UNIT_HEIGHT = 50; //1y = UNIT_HEIGHT px
constexpr int TEXT_AREA_W = 100;
constexpr int COLOR_RECT_W = 80;
constexpr int COLOR_RECT_H = 30;
constexpr int STEP_PER_SAMPLE = 4;

QSize SpectrumWidget::sizeHint() const
{
	return QSize(PADDING*2 + PR::Spectrum::SAMPLING_COUNT * SAMPLE_SPACING,
		PADDING*2 + UNIT_HEIGHT * 2);
}

void SpectrumWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);
	//painter.setRenderHint(QPainter::Antialiasing, true);

	const int mx = width() / 2;
	const int mh = height() / 2;

	const float xspacing = qMax<float>(SAMPLE_SPACING,
			(width() - PADDING * 4 - TEXT_AREA_W) / (float)PR::Spectrum::SAMPLING_COUNT);
	const float yspacing = qMax<float>(UNIT_HEIGHT, mh - PADDING);


	const int w = (PR::Spectrum::SAMPLING_COUNT - 1)*xspacing;
	const int h = yspacing;

	PR::Spectrum spec;	
	if (mSpecMax <= 1)
	{
		spec = mSpectrum;
	}
	else
	{
		spec = mSpectrum.normalized();// Get normalized values
	}

	const float gridY = yspacing / (mSpecMax *2);
	const float gridX = xspacing;

	// Set plot curve
	QPointF points[PR::Spectrum::SAMPLING_COUNT];
	for (PR::uint32 i = 0; i < PR::Spectrum::SAMPLING_COUNT; ++i)
	{
		points[i].setX(PADDING * 2 + i*xspacing);
		points[i].setY(mh - spec.value(i) * yspacing);
	}

	// Grid
	for (PR::uint32 i = 1; i < (PR::uint32)(mSpecMax) * 2; ++i)
	{
		if (i % 2 == 1) // 0.5
		{
			painter.setPen(QPen(Qt::gray, 0.5f, Qt::DotLine));
		}
		else // 1.0
		{
			painter.setPen(QPen(Qt::gray, 1, Qt::DotLine));
		}

		painter.drawLine(PADDING * 2, mh + gridY * i, PADDING * 2 + w, mh + gridY * i);
		painter.drawLine(PADDING * 2, mh - gridY * i, PADDING * 2 + w, mh - gridY * i);
	}

	for (PR::uint32 i = 1; i < PR::Spectrum::SAMPLING_COUNT - 1; ++i)
	{
		painter.setPen(QPen(Qt::lightGray, 0.5f, Qt::DashLine));
		painter.drawLine(PADDING * 2 + i*gridX, mh - h, PADDING * 2 + i*gridX, mh + h);
	}

	// Coord system
	painter.setPen(Qt::black);
	painter.drawLine(PADDING * 2, mh - h, PADDING * 2, mh + h);
	painter.drawLine(PADDING, mh + h, PADDING * 3, mh + h);
	painter.drawLine(PADDING, mh - h, PADDING * 3, mh - h);

	painter.drawText(PADDING * 4, mh - h + 10, QString::number(mSpecMax));

	painter.drawLine(PADDING * 2 + w, mh - h, PADDING * 2 + w, mh + h);
	painter.drawLine(PADDING + w, mh + h, w + PADDING * 3, mh + h);
	painter.drawLine(PADDING + w, mh - h, w + PADDING * 3, mh - h);

	painter.drawLine(PADDING * 2, mh, PADDING * 2 + w, mh);

	// Coord X-Subs
	for (PR::uint32 i = STEP_PER_SAMPLE;
		i < PR::Spectrum::SAMPLING_COUNT - 1;
		i += STEP_PER_SAMPLE)
	{
		painter.drawLine(PADDING * 2 + i*xspacing, mh - 5, PADDING * 2 + i*xspacing, mh + 5);
		painter.drawText(PADDING * 2 + i*xspacing - 10, mh + 20,
			QString::number(PR::Spectrum::WAVELENGTH_START + i*PR::Spectrum::WAVELENGTH_STEP));
	}

	// Function
	painter.setPen(Qt::red);
	painter.drawPolyline(points, PR::Spectrum::SAMPLING_COUNT);

	// Draw selection line
	if (mCurrentNM != -1)
	{
		const float val = mSpectrum.approx(
			PR::Spectrum::WAVELENGTH_START + PR::Spectrum::WAVELENGTH_AREA_SIZE*mCurrentNM) / mSpecMax;

		painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));

		painter.drawLine(PADDING * 2 + mCurrentNM*w, mh - h,
			PADDING * 2 + mCurrentNM*w, mh + h);

		painter.setPen(QPen(Qt::darkRed, 1, Qt::SolidLine));
		painter.setBrush(Qt::red);
		painter.drawEllipse(QPointF(PADDING * 2 + mCurrentNM*w,
			mh - val*yspacing),
			3, 3);

		painter.setPen(Qt::black);

		float dx = mCurrentNM < 0.5f ? 5 : -60;
		if (val > 0)
		{
			painter.drawText(PADDING * 2 + dx + mCurrentNM*w,
				mh + 40, QString::number(val*mSpecMax));
			painter.drawText(PADDING * 2 + dx + mCurrentNM*w,
				mh + 50, QString("%1 nm").arg(
					PR::Spectrum::WAVELENGTH_START + mCurrentNM*PR::Spectrum::WAVELENGTH_AREA_SIZE));
		}
		else
		{
			painter.drawText(PADDING * 2 + dx + mCurrentNM*w,
				mh - 50, QString::number(val*mSpecMax));
			painter.drawText(PADDING * 2 + dx + mCurrentNM*w,
				mh - 40, QString("%1 nm").arg(
					PR::Spectrum::WAVELENGTH_START + mCurrentNM*PR::Spectrum::WAVELENGTH_AREA_SIZE));
		}
	}

	// Draw Labels
	painter.setPen(Qt::black);

	painter.drawText(PADDING * 3 + w, PADDING * 4, mSpectrum.isEmissive() ? tr("Y: Power") : tr("Y: Reflectance"));
	painter.drawText(PADDING * 3 + w, PADDING * 6, tr("X: Wavelength [nm]"));

	// Color
	painter.setBrush(mSpecRGB);
	painter.drawRect(PADDING * 4 + w, PADDING * 10, COLOR_RECT_W, COLOR_RECT_H);

	painter.setBrush(mSpecRGBLinear);
	painter.drawRect(PADDING * 4 + w, PADDING * 11 + COLOR_RECT_H, COLOR_RECT_W, COLOR_RECT_H);

	painter.setBrush(mSpecXYZ);
	painter.drawRect(PADDING * 4 + w, PADDING * 12 + COLOR_RECT_H*2, COLOR_RECT_W, COLOR_RECT_H);

	painter.setBrush(mSpecXYZNorm);
	painter.drawRect(PADDING * 4 + w, PADDING * 13 + COLOR_RECT_H*3, COLOR_RECT_W, COLOR_RECT_H);

	// Color labels
	/*painter.save();
	painter.rotate(90);
	painter.drawText(PADDING * 4 + w + COLOR_RECT_W, PADDING * 10, tr("sRGB"));
	painter.rotate(-90);
	painter.restore();*/
}

void SpectrumWidget::mouseMoveEvent(QMouseEvent* event)
{
	const float xspacing = qMax<float>(SAMPLE_SPACING,
		(width() - PADDING * 4 - TEXT_AREA_W) / (float)PR::Spectrum::SAMPLING_COUNT);
	const int w = (PR::Spectrum::SAMPLING_COUNT - 1)*xspacing;

	float x = (event->pos().x() - PADDING*2) / (float)w;

	if (x < 0 || x > 1)
	{
		mCurrentNM = -1;
	}
	else
	{
		mCurrentNM = x;
	}

	repaint();
}

void SpectrumWidget::cache()
{
	mSpecMax = qMax(1.0f, qMax(mSpectrum.max(), -mSpectrum.min()));

	float R, G, B;
	PR::RGBConverter::convertGAMMA(mSpectrum, R, G, B);
	mSpecRGB = QColor(qBound<int>(0, R * 255, 255),
		qBound<int>(0, G * 255, 255),
		qBound<int>(0, B * 255, 255));

	PR::RGBConverter::convert(mSpectrum, R, G, B);
	mSpecRGBLinear = QColor(qBound<int>(0, R * 255, 255),
		qBound<int>(0, G * 255, 255),
		qBound<int>(0, B * 255, 255));

	PR::XYZConverter::convertXYZ(mSpectrum, R, G, B);
	mSpecXYZ = QColor(qBound<int>(0, R * 255, 255),
		qBound<int>(0, G * 255, 255),
		qBound<int>(0, B * 255, 255));

	PR::XYZConverter::convert(mSpectrum, R, G, B);
	mSpecXYZNorm = QColor(qBound<int>(0, R * 255, 255),
		qBound<int>(0, G * 255, 255),
		qBound<int>(0, B * 255, 255));
}