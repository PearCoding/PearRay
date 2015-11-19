#pragma once

#include <QWidget>
#include <QImage>

#include "spectral/Spectrum.h"

class SpectrumWidget : public QWidget
{
	Q_OBJECT
public:
	SpectrumWidget(QWidget *parent);
	~SpectrumWidget();

	inline PR::Spectrum spectrum() const
	{
		return mSpectrum;
	}

	QSize sizeHint() const override;

public slots:
	inline void setSpectrum(const PR::Spectrum& spec)
	{
		mSpectrum = spec;
		cache();
		repaint();
	}

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	void cache();

	PR::Spectrum mSpectrum;
	float mCurrentNM;

	// Cache
	float mSpecMax;
	QColor mSpecRGB;
	QColor mSpecRGBLinear;
	QColor mSpecXYZ;
	QColor mSpecXYZNorm;
};
