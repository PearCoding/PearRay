#pragma once

#include <QWidget>
#include <QImage>

namespace PR
{
	class Renderer;
}

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent);
	~ViewWidget();

	void setRenderer(PR::Renderer* renderer);

public slots:
	void enableScale(bool b);

	void refreshView();

protected:
	virtual void paintEvent(QPaintEvent* event);

private:
	PR::Renderer* mRenderer;

	bool mScale;
	QImage mRenderImage;
};
