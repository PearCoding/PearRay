#include "ViewWidget.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"

#include "spectral/ToneMapper.h"
#include "PearMath.h"

#include "Logger.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

ViewWidget::ViewWidget(QWidget *parent)
	: QWidget(parent),
	mRenderer(nullptr), mToolMode(TM_Selection),
	mDisplayMode1D(PR::OutputMap::V_1D_COUNT), mDisplayMode3D(PR::OutputMap::V_3D_COUNT),
	mShowProgress(true),
	mRenderData(nullptr), mToneMapper(nullptr),
	mZoom(1), mPanX(0), mPanY(0), mLastPanX(0), mLastPanY(0), mPressing(false)
{
	cache();
	cacheScale();
	setToolMode(TM_Selection);
}

ViewWidget::~ViewWidget()
{
	if (mRenderData)
	{
		delete mToneMapper;
		delete[] mRenderData;
	}
}

void ViewWidget::setRenderer(PR::RenderContext* renderer)
{
	mRenderer = renderer;

	if (mRenderData)
	{
		delete mToneMapper;
		mToneMapper = nullptr;

		delete[] mRenderData;
		mRenderData = nullptr;
	}

	if (mRenderer)
	{
		mRenderData = new float[mRenderer->width()*mRenderer->height()*3];
		std::memset(mRenderData, 0,
			mRenderer->width()*mRenderer->height() * 3 * sizeof(uchar));

		mToneMapper = new PR::ToneMapper(mRenderer->gpu(),
			mRenderer->width()*mRenderer->height());
	}

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
	if(mRenderImage.isNull())
	{
		mZoom = 1;
	}
	else
	{
		QSize newSize = mRenderImage.size().scaled(size(), Qt::KeepAspectRatio);

		if(newSize.width() > newSize.height())
			mZoom = newSize.width() / (float)mRenderImage.width();
		else
			mZoom = newSize.height() / (float)mRenderImage.height();
	}

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
	else if (mToolMode == TM_Crop)
	{
		setCursor(QCursor(Qt::CrossCursor));
	}

	repaint();
}

void ViewWidget::setDisplayMode(quint32 mode)
{
	if(mode == 0)// Spectral
	{
		mDisplayMode1D = PR::OutputMap::V_1D_COUNT;// OFF
		mDisplayMode3D = PR::OutputMap::V_3D_COUNT;// OFF
	}
	else if(mode < PR::OutputMap::V_3D_COUNT + 1)
	{
		mDisplayMode1D = PR::OutputMap::V_1D_COUNT;// OFF
		mDisplayMode3D = (PR::OutputMap::Variable3D)(mode - 1);
	}
	else
	{
		mDisplayMode1D = (PR::OutputMap::Variable1D)(mode - 1 - PR::OutputMap::V_3D_COUNT);
		mDisplayMode3D = PR::OutputMap::V_3D_COUNT;// OFF
	}

	refreshView();
}

void ViewWidget::setCropSelection(const QPoint& start, const QPoint& end)
{
	mCropStart = start;
	mCropEnd = end;

	if(mToolMode == TM_Crop)
		repaint();
}

QRect ViewWidget::selectedCropRect() const
{
	return QRect(mCropStart, mCropEnd).normalized();
}

QPoint ViewWidget::convertToLocal(const QPoint& p)
{
	return (p - QPoint(width() / 2, height() / 2) + QPoint(mPanX, mPanY))/mZoom +
		QPoint(mRenderImage.width()/2, mRenderImage.height()/2);
}

QPoint ViewWidget::convertToGlobal(const QPoint& p)
{
	return p*mZoom + QPoint(width() / 2, height() / 2) - QPoint(mPanX, mPanY) -
		QPoint(mScaledImage.width() / 2, mScaledImage.height() / 2);
}

void ViewWidget::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (mToolMode == TM_Selection)
		{
			QPoint p = convertToLocal(event->pos());
			if (QRect(QPoint(0, 0), mRenderImage.size()).contains(p) &&
				p.x() >= mRenderer->offsetX() && p.x() < mRenderer->offsetX() + mRenderer->width()&&
				p.y() >= mRenderer->offsetY() && p.y() < mRenderer->offsetY() + mRenderer->height())
			{
				emit spectrumSelected(
					mRenderer->output()->getSpectralChannel()->getFragment(p.x(), p.y()));
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
		else if (mToolMode == TM_Crop)
		{
			mPressing = true;
			mCropEnd = convertToLocal(event->pos());
			mCropStart = mCropEnd;
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
			QPoint pan = convertToLocal(mStartPos) - QPoint(mScaledImage.width() / 2, mScaledImage.height() / 2) + QPoint(width() / 2, height() / 2) - QPoint(mPanX, mPanY);

			float zoom;
			if (newSize.width() > newSize.height())
				zoom = oldSize.width() / (float)newSize.width();
			else
				zoom = oldSize.height() / (float)newSize.height();

			mZoom = qMin(qMax(zoom*mZoom, 0.001f), 50.0f);

			mPanX = pan.x();
			mPanY = pan.y();

			mLastPos = event->pos();
			cacheScale();
			event->accept();
		}
		else if (mToolMode == TM_Crop)
		{
			mCropEnd = convertToLocal(event->pos());
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
			mLastPos = event->pos();
			repaint();
			event->accept();
		}
		else if (mToolMode == TM_Crop)
		{
			mCropEnd = convertToLocal(event->pos());
			repaint();
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

	QRect cropRect = QRect(convertToGlobal(mCropStart), convertToGlobal(mCropEnd)).normalized();
	if (mToolMode == TM_Crop &&
		cropRect.isValid() && cropRect.width() > 2 && cropRect.height() > 2)
	{
		painter.setBrush(QColor(255, 255, 0, 150));
		painter.setPen(Qt::darkYellow);

		painter.drawRect(cropRect);
	}

	if (mShowProgress && mRenderer && !mRenderer->isFinished())
	{
		painter.setBrush(Qt::transparent);
		painter.setPen(Qt::darkRed);

		for(PR::RenderTile tile : mRenderer->currentTiles())
		{
			QRect rect = QRect(convertToGlobal(QPoint(tile.sx(), tile.sy())),
				convertToGlobal(QPoint(tile.ex(), tile.ey())));
			painter.drawRect(rect);
		}
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
		QImage src;
		bool success = false;
		PR::OutputMap* map = mRenderer->output();
		
		if(map)
		{
			if(mDisplayMode1D == PR::OutputMap::V_1D_COUNT &&
				mDisplayMode3D == PR::OutputMap::V_3D_COUNT)// Spectral
			{
				mToneMapper->exec(map->getSpectralChannel()->ptr(), mRenderData);

				uchar* tmp = new uchar[mRenderer->width() * mRenderer->height() * 3];
				for(int i = 0; mRenderer->width()*mRenderer->height()*3; ++i)
					tmp[i] = static_cast<uchar>(mRenderData[i]*255);

				src = QImage(tmp, mRenderer->width(), mRenderer->height(), QImage::Format_RGB888);
				delete[] tmp;
				success = true;
			}
			else if(mDisplayMode1D != PR::OutputMap::V_1D_COUNT)
			{
				PR::Output1D* channel = map->getChannel(mDisplayMode1D);

				if(channel)
				{
					for(int i = 0; i < mRenderer->width() * mRenderer->height(); ++i)
					{
						mRenderData[i*3] = channel->ptr()[i];
						mRenderData[i*3 + 1] = channel->ptr()[i];
						mRenderData[i*3 + 2] = channel->ptr()[i];
					}

					mToneMapper->execMapper(mRenderData, mRenderData);

					uchar* tmp = new uchar[mRenderer->width()*mRenderer->height()];
					for(int i = 0; mRenderer->width()*mRenderer->height(); ++i)
						tmp[i] = static_cast<uchar>(mRenderData[i*3]*255);

					src = QImage(tmp, mRenderer->width(), mRenderer->height(), QImage::Format_Grayscale8);
					delete[] tmp;
					success = true;
				}
			}
			else
			{
				PR::Output3D* channel = map->getChannel(mDisplayMode3D);

				if(channel)
				{
					for(int i = 0; mRenderer->width()*mRenderer->height(); ++i)
					{
						const PM::avec3& a = channel->ptr()[i];
						mRenderData[i*3] = a[0];
						mRenderData[i*3+1] = a[1];
						mRenderData[i*3+2] = a[2];
					}

					mToneMapper->execMapper(mRenderData, mRenderData);

					uchar* tmp = new uchar[mRenderer->width()*mRenderer->height()];
					for(int i = 0; mRenderer->width()*mRenderer->height()*3; ++i)
						tmp[i] = static_cast<uchar>(mRenderData[i]*255);

					src = QImage(tmp, mRenderer->width(), mRenderer->height(), QImage::Format_RGB888);
					delete[] tmp;
					success = true;
				}
			}
		}
		
		mRenderImage = QImage(mRenderer->fullWidth(), mRenderer->fullHeight(), QImage::Format_RGB888);
		mRenderImage.fill(Qt::black);
		if(success)
		{
			QPainter painter(&mRenderImage);
			painter.drawImage(QPoint(mRenderer->offsetX(), mRenderer->offsetY()), src);
			painter.end();
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
		mScaledImage = mRenderImage.scaled(mRenderImage.width()*mZoom, mRenderImage.height()*mZoom,
			Qt::IgnoreAspectRatio, Qt::FastTransformation);
}