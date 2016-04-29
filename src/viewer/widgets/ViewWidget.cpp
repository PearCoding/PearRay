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
	mRenderer(nullptr), mViewMode(VM_ToneMapped), mScale(false)
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
		const PR::RenderResult& result = mRenderer->result();

		mRenderImage = QImage(result.width(), result.height(), QImage::Format_RGB888);

		if (mViewMode == VM_ToneMapped)// Very simple tone mapper...
		{
			constexpr float STRENGTH = 1 / 9.0f;
			/*constexpr float FILTER[] = { STRENGTH, STRENGTH, STRENGTH,
				STRENGTH, STRENGTH, STRENGTH,
				STRENGTH, STRENGTH, STRENGTH
			};*/
			/*constexpr float FILTER[] = { -STRENGTH, -STRENGTH, -STRENGTH,
				-STRENGTH, 8*STRENGTH, -STRENGTH,
				-STRENGTH, -STRENGTH, -STRENGTH
			};*/
			constexpr int MEDIAN_RADIUS = 3;
			constexpr int MEDIAN_WIDTH = 2 * MEDIAN_RADIUS + 1;
			constexpr int MEDIAN_SIZE = MEDIAN_WIDTH * MEDIAN_WIDTH;

			constexpr float RHO = 0.45f;
			constexpr float RHO2 = RHO * RHO;

			// Gaussian
			float FILTER[MEDIAN_SIZE];
			for (int i = 0; i < MEDIAN_WIDTH; ++i)
			{
				for (int j = 0; j < MEDIAN_WIDTH; ++j)
				{
					float rx = i - MEDIAN_RADIUS;
					float ry = j - MEDIAN_RADIUS;
					FILTER[j * MEDIAN_WIDTH + i] = (0.5f * PM_INV_PI_F / RHO2) * std::exp(-0.5f*(rx*rx + ry*ry) / RHO2);
				}
			}

			/*PR::Spectrum window[MEDIAN_SIZE];
			float maxes[MEDIAN_SIZE];*/

			PR::RenderResult tmp(result.width(), result.height());
			float max = 0;
			for (PR::uint32 y = 0; y < result.height(); ++y)
			{
				for (PR::uint32 x = 0; x < result.width(); ++x)
				{
					if (x >= MEDIAN_RADIUS && x < result.width() - MEDIAN_RADIUS &&
						y >= MEDIAN_RADIUS && y < result.height() - MEDIAN_RADIUS)
					{
						PR::Spectrum spec;
						for (int i = 0; i < MEDIAN_WIDTH; ++i)
						{
							for (int j = 0; j < MEDIAN_WIDTH; ++j)
							{
								spec += result.point(x + i - MEDIAN_RADIUS, y + j - MEDIAN_RADIUS) * FILTER[j * MEDIAN_WIDTH + i];
							}
						}

						max = PM::pm_MaxT(max, spec.max());
						tmp.setPoint(x, y, spec);

						// Median Filter
						/*for (int i = 0; i < MEDIAN_WIDTH; ++i)
						{
							for (int j = 0; j < MEDIAN_WIDTH; ++j)
							{
								auto in = result.point(x + i - MEDIAN_RADIUS, y + j - MEDIAN_RADIUS);

								int index = j * MEDIAN_WIDTH + i;

								if (index == 0)
								{
									window[index] = in;
									maxes[index] = in.max();
								}
								else
								{
									float inMax = in.max();
									int insertIndex;
									for (insertIndex = 0; insertIndex < index; ++insertIndex)
									{
										if (maxes[index] > inMax)
											break;
									}

									for (int k = index - 1; k >= insertIndex; --k)
									{
										maxes[k + 1] = maxes[k];
										window[k + 1] = window[k];
									}

									maxes[insertIndex] = inMax;
									window[insertIndex] = in;
								}
							}
						}

						max = PM::pm_MaxT(max, maxes[MEDIAN_SIZE / 2]);
						tmp.setPoint(x, y, window[MEDIAN_SIZE / 2]);*/
					}
					else
					{
						PR::Spectrum spec = result.point(x, y);
						max = PM::pm_MaxT(max, spec.max());
						tmp.setPoint(x, y, spec);
					}
				}
			}

			float factor = 1;/*(max <= 0.8f) ? 1 : 1 / max;*/
			for (PR::uint32 y = 0; y < tmp.height(); ++y)
			{
				for (PR::uint32 x = 0; x < tmp.width(); ++x)
				{
					float r;
					float g;
					float b;

					PR::RGBConverter::convertGAMMA(tmp.point(x, y)*factor, r, g, b);
					r = PM::pm_ClampT<float>(r, 0, 1);
					g = PM::pm_ClampT<float>(g, 0, 1);
					b = PM::pm_ClampT<float>(b, 0, 1);

					mRenderImage.setPixel(x, y, qRgb(r * 255, g * 255, b * 255));
				}
			}
		}
		else if (mViewMode == VM_Color)
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