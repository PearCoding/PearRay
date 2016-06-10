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
	VM_ToneMapped,// sRGB
	VM_Color,// sRGB
	VM_ColorLinear,// sRGB Linear
	VM_XYZ,// Color space (CIE XYZ)
	VM_NORM_XYZ// Color space (CIE XYZ)
};

enum ToolMode
{
	TM_Selection,
	TM_Pan,
	TM_Zoom
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

	void cache();
	void cacheScale();

	PR::Renderer* mRenderer;

	ViewMode mViewMode;
	ToolMode mToolMode;

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
};
