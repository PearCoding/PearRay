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

	QSize minimumSizeHint() const override;

public slots:
	inline void setSpectrum(const PR::Spectrum& spec)
	{
		mSpectrum = spec;
		cache();
		cacheImage();
		repaint();
	}

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	void cache();
	void cacheImage();

	PR::Spectrum mSpectrum;
	float mCurrentNM;

	// Cache
	QPixmap mCache;

	bool mSpecInf;
	bool mSpecNaN;
	float mSpecMax;
	QColor mSpecRGB;
	QColor mSpecRGBLinear;
	QColor mSpecXYZ;
	QColor mSpecXYZNorm;
};
