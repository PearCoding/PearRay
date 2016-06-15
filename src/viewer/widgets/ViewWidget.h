#pragma once

#include <QWidget>
#include <QImage>

namespace PR
{
	class Renderer;
	class Spectrum;
}

enum ViewMode
{
	VM_Color,// sRGB
	VM_ColorLinear,// sRGB Linear
	VM_XYZ,// Color space (CIE XYZ)
	VM_NORM_XYZ// Color space (CIE XYZ)
};

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

	inline ViewMode viewMode() const
	{
		return mViewMode;
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
	inline void setViewMode(ViewMode vm)
	{
		mViewMode = vm;
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

	ViewMode mViewMode;
	ToolMode mToolMode;

	bool mShowProgress;

	QImage mRenderImage;
	QImage mScaledImage;

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
