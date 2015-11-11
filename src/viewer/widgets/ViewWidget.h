#pragma once

#include <QWidget>
#include <QImage>

namespace PR
{
	class Renderer;
}

enum ViewMode
{
	VM_Color,
	VM_Depth
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

public slots:
	void enableScale(bool b);

	void refreshView();

protected:
	virtual void paintEvent(QPaintEvent* event);

private:
	PR::Renderer* mRenderer;

	ViewMode mViewMode;
	bool mScale;
	QImage mRenderImage;
};
