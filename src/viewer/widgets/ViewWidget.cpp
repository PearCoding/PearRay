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
	mRenderer(nullptr), mViewMode(VM_Color), mToolMode(TM_Selection),
	mZoom(1), mPanX(0), mPanY(0), mLastPanX(0), mLastPanY(0), mPressing(false)
{
	cache();
	cacheScale();
	setToolMode(TM_Selection);
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setRenderer(PR::Renderer* renderer)
{
	mRenderer = renderer;
	refreshView();
}

void ViewWidget::resetZoomPan()
{
	mZoom = 1;
	mPanX = 0;
	mPanY = 0;

	cacheScale();
	repaint();
}

void ViewWidget::fitIntoWindow()
{
	QSize newSize = mRenderImage.size().scaled(size(), Qt::KeepAspectRatio);

	if(newSize.width() > newSize.height())
		mZoom = newSize.width() / (float)mRenderImage.width();
	else
		mZoom = newSize.height() / (float)mRenderImage.height();

	mPanX = 0;
	mPanY = 0;

	cacheScale();
	repaint();
}

void ViewWidget::zoomIn()
{
	mZoom *= 1.1f;
	cacheScale();
	repaint();
}

void ViewWidget::zoomOut()
{
	mZoom *= 0.91f;
	cacheScale();
	repaint();
}

void ViewWidget::setToolMode(ToolMode tm)
{
	mToolMode = tm;

	if (mToolMode == TM_Selection)
	{
		setCursor(QCursor(Qt::CrossCursor));
	}
	else if (mToolMode == TM_Pan)
	{
		setCursor(QCursor(Qt::OpenHandCursor));
	}
	else if (mToolMode == TM_Zoom)
	{
		setCursor(QCursor(QPixmap(":/zoom_cursor.png")));
	}
}

void ViewWidget::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (mToolMode == TM_Selection)
		{
			int x = width() / 2 - mZoom * mRenderImage.width() / 2 - mPanX;
			int y = height() / 2 - mZoom * mRenderImage.height() / 2 - mPanY;

			QRect rect(x, y, mRenderImage.width(), mRenderImage.height());
			if (rect.contains(event->pos()))
			{
				PR::RenderResult result = mRenderer->result();
				emit spectrumSelected(result.point(event->pos().x() - x, event->pos().y() - y));
			}
		}
		else if (mToolMode == TM_Pan)
		{
			mPressing = true;
			mLastPanX = mPanX;
			mLastPanY = mPanY;
			mLastPos = event->pos();
			setCursor(QCursor(Qt::ClosedHandCursor));
			event->accept();
		}
		else if (mToolMode == TM_Zoom)
		{
			mPressing = true;
			mLastPos = event->pos();
			mStartPos = mLastPos;
			event->accept();
		}

		event->accept();
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		mPressing = false;

		if (mToolMode == TM_Pan)
		{
			mLastPos = event->pos();
			mLastPanX = mPanX;
			mLastPanY = mPanY;
			setCursor(QCursor(Qt::OpenHandCursor));
			event->accept();
		}
		else if (mToolMode == TM_Zoom)
		{
			//TODO: Add pan!
			QSize oldSize = mRenderImage.size();
			QSize newSize = oldSize.scaled(QRect(mStartPos, mLastPos).size(), Qt::KeepAspectRatio);

			float zoom;
			if (newSize.width() > newSize.height())
				zoom = oldSize.width() / (float)newSize.width();
			else
				zoom = oldSize.height() / (float)newSize.height();

			mZoom = qMin(qMax(zoom*mZoom, 0.001f), 50.0f);

			mLastPos = event->pos();
			cacheScale();
			event->accept();
		}
		repaint();
	}
}

constexpr float DeltaPan = 0.5f;
void ViewWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (mToolMode == TM_Pan)
		{
			const float d = DeltaPan / mZoom;
			QPointF pos = event->pos() - mLastPos;

			mPanX = mLastPanX - pos.x()*d;
			mPanY = mLastPanY - pos.y()*d;

			repaint();
			event->accept();
		}
		else if (mToolMode == TM_Zoom)
		{
			repaint();
			mLastPos = event->pos();
			event->accept();
		}
	}
}

void ViewWidget::wheelEvent(QWheelEvent * event)
{
	if (event->orientation() == Qt::Vertical)
	{
		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15;

		if (numDegrees >= 1)
			mZoom *= std::pow(1.1f,numSteps);
		else if (numDegrees <= -1)
			mZoom *= std::pow(0.91f, -numSteps);

		mZoom = qMax(qMin(mZoom, 50.0f), 0.01f);

		event->accept();
		cacheScale();
		repaint();
	}
}

void ViewWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, mBackgroundImage);
	
	if (!mScaledImage.isNull())
	{
		int x = width() / 2 - mScaledImage.width() / 2 - mPanX;
		int y = height() / 2 - mScaledImage.height() / 2 - mPanY;
		painter.drawImage(x, y, mScaledImage);
	}

	if (mToolMode == TM_Zoom && mPressing)
	{
		painter.setBrush(QColor(0, 128, 255, 128));
		painter.setPen(Qt::darkBlue);

		painter.drawRect(QRect(mStartPos, mLastPos));
	}
}

void ViewWidget::resizeEvent(QResizeEvent* event)
{
	cache();
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

			float factor = (!std::isnormal(max) || max <= 0.9f) ? 1 : 1 / max;
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

	cacheScale();
	repaint();
}

constexpr int RECT_SIZE = 16;

void ViewWidget::cache()
{
	QImage image = QImage(width(), height(), QImage::Format_RGB888);

	QPainter painter(&image);
	painter.setPen(Qt::NoPen);

	uint32_t xc = width() / RECT_SIZE + 1;
	uint32_t yc = height() / RECT_SIZE + 1;

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

void ViewWidget::cacheScale()
{
	if(!mRenderImage.isNull())
		mScaledImage = mRenderImage.scaled(mRenderImage.width()*mZoom, mRenderImage.height()*mZoom);
}