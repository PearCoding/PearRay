#pragma once

#include <QWidget>
#include <QImage>

#include "spectral/ToneMapper.h"

namespace PR
{
	class Renderer;
	class Spectrum;
	class ToneMapper;
}

enum ToolMode
{
	TM_Selection,
	TM_Pan,
	TM_Zoom,
	TM_Crop
};

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent);
	~ViewWidget();

	void setRenderer(PR::Renderer* renderer);

	inline PR::ToneColorMode colorMode() const
	{
		return mToneMapper->colorMode();
	}

	inline PR::ToneGammaMode gammaMode() const
	{
		return mToneMapper->gammaMode();
	}

	inline PR::ToneMapperMode mapperMode() const
	{
		return mToneMapper->mapperMode();
	}

	inline ToolMode toolMode() const
	{
		return mToolMode;
	}

	inline QImage image() const
	{
		return mRenderImage;
	}

	void setCropSelection(const QPoint& start, const QPoint& end);
	QRect selectedCropRect() const;

public slots:
	inline void setColorMode(PR::ToneColorMode mode)
	{
		mToneMapper->setColorMode(mode);
		refreshView();
	}

	inline void setGammaMode(PR::ToneGammaMode mode)
	{
		mToneMapper->setGammaMode(mode);
		refreshView();
	}

	inline void setMapperMode(PR::ToneMapperMode mode)
	{
		mToneMapper->setMapperMode(mode);
		refreshView();
	}

	void setToolMode(ToolMode tm);
	
	void resetZoomPan();
	void fitIntoWindow();

	void refreshView();

	void zoomIn();
	void zoomOut();

signals:
	void spectrumSelected(const PR::Spectrum& spec);

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent * event) override;
	virtual void mouseMoveEvent(QMouseEvent * event) override;
	virtual void wheelEvent(QWheelEvent * event) override;

private:
	QPoint convertToLocal(const QPoint& p);
	QPoint convertToGlobal(const QPoint& p);

	void cache();
	void cacheScale();

	PR::Renderer* mRenderer;

	ToolMode mToolMode;

	bool mShowProgress;

	uchar* mRenderData;
	QImage mRenderImage;
	QImage mScaledImage;

	PR::ToneMapper* mToneMapper;

	QPixmap mBackgroundImage;

	float mZoom;
	float mPanX;
	float mPanY;

	float mLastPanX;
	float mLastPanY;

	QPoint mStartPos;
	QPoint mLastPos;

	bool mPressing;

	QPoint mCropStart;
	QPoint mCropEnd;
};
