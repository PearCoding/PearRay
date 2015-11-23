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
	VM_Depth,
	VM_XYZ,// Color space (CIE XYZ)
	VM_NORM_XYZ// Color space (CIE XYZ)
};
class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent);
	~ViewWidget();

	void setRenderer(PR::Renderer* renderer);

	inline void setViewMode(ViewMode vm)
	{
		mViewMode = vm;
		refreshView();
	}

	inline ViewMode viewMode() const
	{
		return mViewMode;
	}

	inline QImage image() const
	{
		return mRenderImage;
	}

public slots:
	void enableScale(bool b);
	void refreshView();

signals:
	void spectrumSelected(const PR::Spectrum& spec);

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;

private:
	void cache();

	PR::Renderer* mRenderer;

	ViewMode mViewMode;
	bool mScale;
	QImage mRenderImage;

	QPixmap mBackgroundImage;
};
