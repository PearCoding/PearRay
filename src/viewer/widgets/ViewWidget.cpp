#include "ViewWidget.h"

#include "renderer/Renderer.h"
#include "renderer/RenderResult.h"

#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"
#include "PearMath.h"

#include <QPainter>

ViewWidget::ViewWidget(QWidget *parent)
	: QWidget(parent),
	mRenderer(nullptr), mViewMode(VM_Color), mScale(false)
{

}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setRenderer(PR::Renderer* renderer)
{
	mRenderer = renderer;
	refreshView();
}

void ViewWidget::enableScale(bool b)
{
	mScale = b;
	repaint();
}

void ViewWidget::refreshView()
{
	if (mRenderer)
	{
		PR::RenderResult result = mRenderer->result();

		mRenderImage = QImage(result.width(), result.height(), QImage::Format_RGB888);

		if (mViewMode == VM_Color)
		{
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					float r;
					float g;
					float b;

					PR::RGBConverter::convert(result.point(x, y), r, g, b);
					r = PM::pm_MinT<float>(1, r);
					g = PM::pm_MinT<float>(1, g);
					b = PM::pm_MinT<float>(1, b);

					mRenderImage.setPixel(x, y, qRgb(r * 255, g * 255, b * 255));
				}
			}
		}
		else if (mViewMode == VM_Depth)
		{
			float maxDepth = result.maxDepth();
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					float d = result.depth(x, y) / maxDepth;
					d = d < 0 ? 0 : 1 - d;
					mRenderImage.setPixel(x, y, qRgb(d*255, d*255, d*255));
				}
			}
		}
		else if (mViewMode == VM_XYZ)
		{
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					float r;
					float g;
					float b;

					PR::XYZConverter::convertXYZ(result.point(x, y), r, g, b);

					float m = PM::pm_MaxT<float>(1, PM::pm_MaxT<float>(r, PM::pm_MaxT<float>(g, b)));
					if (m != 0)
					{
						r /= m;
						g /= m;
						b /= m;
					}

					mRenderImage.setPixel(x, y, qRgb(r * 255, g * 255, b * 255));
				}
			}
		}
		else if (mViewMode == VM_NORM_XYZ)
		{
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					float r;
					float g;
					float b;

					PR::XYZConverter::convert(result.point(x, y), r, g, b);

					mRenderImage.setPixel(x, y, qRgb(r * 255, g * 255, b * 255));
				}
			}
		}
	}
	else
	{
		mRenderImage = QImage();
	}

	repaint();
}

void ViewWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::magenta);

	QImage img;
	if (mScale)
	{
		img = mRenderImage.scaled(size(), Qt::KeepAspectRatio);
	}
	else
	{
		img = mRenderImage;
	}

	int x = width() / 2 - img.width() / 2;
	int y = height() / 2 - img.height() / 2;
	painter.drawImage(x, y, img);
}