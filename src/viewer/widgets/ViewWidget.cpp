#include "ViewWidget.h"

#include "renderer/Renderer.h"
#include "renderer/RenderResult.h"

#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"
#include "PearMath.h"

#include <QPainter>
#include <QMouseEvent>

ViewWidget::ViewWidget(QWidget *parent)
	: QWidget(parent),
	mRenderer(nullptr), mViewMode(VM_Color), mScale(false)
{
	cache();
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

					PR::RGBConverter::convertGAMMA(result.point(x, y), r, g, b);
					r = PM::pm_ClampT<float>(r, 0, 1);
					g = PM::pm_ClampT<float>(g, 0, 1);
					b = PM::pm_ClampT<float>(b, 0, 1);

					mRenderImage.setPixel(x, y, qRgb(r * 255, g * 255, b * 255));
				}
			}
		}
		else if (mViewMode == VM_ColorLinear)
		{
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					float r;
					float g;
					float b;

					PR::RGBConverter::convert(result.point(x, y), r, g, b);
					r = PM::pm_ClampT<float>(r, 0, 1);
					g = PM::pm_ClampT<float>(g, 0, 1);
					b = PM::pm_ClampT<float>(b, 0, 1);

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
					mRenderImage.setPixel(x, y, qRgb(d * 255, d * 255, d * 255));
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
	painter.drawPixmap(0, 0, mBackgroundImage);

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

void ViewWidget::mousePressEvent(QMouseEvent * event)
{
	int x = width() / 2 - mRenderImage.width() / 2;
	int y = height() / 2 - mRenderImage.height() / 2;

	QRect rect(x, y, mRenderImage.width(), mRenderImage.height());
	if (rect.contains(event->pos()))
	{
		PR::RenderResult result = mRenderer->result();
		emit spectrumSelected(result.point(event->pos().x() - x, event->pos().y() - y));
	}
}

void ViewWidget::resizeEvent(QResizeEvent* event)
{
	cache();
}

constexpr int RECT_SIZE = 16;

void ViewWidget::cache()
{
	QImage image = QImage(width(), height(), QImage::Format_RGB888);

	QPainter painter(&image);
	painter.setPen(Qt::NoPen);

	int xc = width() / RECT_SIZE + 1;
	int yc = height() / RECT_SIZE + 1;

	for (uint32_t y = 0; y < yc; ++y)
	{
		for (uint32_t x = 0; x < xc; ++x)
		{
			QColor color;

			if (y % 2)
			{
				if (x % 2)
				{
					color = Qt::gray;
				}
				else
				{
					color = Qt::lightGray;
				}
			}
			else
			{
				if (x % 2)
				{
					color = Qt::lightGray;
				}
				else
				{
					color = Qt::gray;
				}
			}

			painter.setBrush(color);
			painter.drawRect(x*RECT_SIZE, y*RECT_SIZE, RECT_SIZE, RECT_SIZE);
		}
	}

	mBackgroundImage.convertFromImage(image);
}