#include "SpectrumWidget.h"

#include <QPainter>

SpectrumWidget::SpectrumWidget(QWidget *parent)
	: QWidget(parent),
	mSpectrum()
{

}

SpectrumWidget::~SpectrumWidget()
{
}

constexpr int PADDING = 5;
constexpr int SAMPLE_SPACING = 5;
constexpr int UNIT_HEIGHT = 50; //1y = UNIT_HEIGHT px

QSize SpectrumWidget::sizeHint() const
{
	return QSize(PADDING*2 + PR::Spectrum::SAMPLING_COUNT * SAMPLE_SPACING,
		PADDING*2 + UNIT_HEIGHT * 2);
}

void SpectrumWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);

	const int mx = width() / 2;
	const int mh = height() / 2;

	const float xspacing = qMax<float>(SAMPLE_SPACING,
			(width() - PADDING * 4) / (float)PR::Spectrum::SAMPLING_COUNT);
	const float yspacing = qMax<float>(UNIT_HEIGHT, mh - PADDING);


	const int w = (PR::Spectrum::SAMPLING_COUNT - 1)*xspacing;
	const int h = yspacing;

	PR::Spectrum spec;	
	float max = qMax(1.0f, qMax(mSpectrum.max(), -mSpectrum.min()));

	if (max <= 1)
	{
		spec = mSpectrum;
	}
	else
	{
		spec = mSpectrum.normalized();// Get normalized values
	}

	const float gridY = yspacing / (max*2);
	const float gridX = xspacing;

	// Set plot curve
	QPointF points[PR::Spectrum::SAMPLING_COUNT];
	for (PR::uint32 i = 0; i < PR::Spectrum::SAMPLING_COUNT; ++i)
	{
		points[i].setX(PADDING * 2 + i*xspacing);
		points[i].setY(mh - spec.value(i) * yspacing);
	}

	// Grid
	for (PR::uint32 i = 1; i < (PR::uint32)(max) * 2; ++i)
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

	painter.drawLine(PADDING * 2 + w, mh - h, PADDING * 2 + w, mh + h);
	painter.drawLine(PADDING + w, mh + h, w + PADDING * 3, mh + h);
	painter.drawLine(PADDING + w, mh - h, w + PADDING * 3, mh - h);

	painter.drawLine(PADDING * 2, mh, PADDING * 2 + w, mh);

	painter.setPen(Qt::red);
	painter.drawPolyline(points, PR::Spectrum::SAMPLING_COUNT);
}