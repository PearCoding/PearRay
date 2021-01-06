#include "ImageView.h"
#include "io/ImageBufferView.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <QMenu>

namespace PR {
namespace UI {
constexpr int BAR_POS_W = 100;
constexpr int MIN_S		= 400;

constexpr float PAN_W  = 0.4f;
constexpr float ZOOM_W = 1.05f;

// Grid size in inches
constexpr int GRID_S = 20;

ImageView::ImageView(QWidget* parent)
	: QWidget(parent)
	, mChannelMask(0xFF)
	, mChannelOffset(0)
	, mLastPixel(0, 0)
	, mShowUpdateRegions(true)
{
	zoomToOriginal();

	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);

	connect(this, &ImageView::customContextMenuRequested,
			this, &ImageView::showContextMenu);
}

ImageView::~ImageView()
{
}

void ImageView::zoomToFit()
{
	if (mImage.isNull())
		return;

	// Scale
	const float zw	 = viewSize().width() / (float)mImage.width();
	const float zh	 = viewSize().height() / (float)mImage.height();
	const float zoom = std::min(zw, zh);

	mTransform	  = QTransform::fromScale(zoom, zoom);
	mInvTransform = QTransform::fromScale(1 / zoom, 1 / zoom);

	// Shift to center
	centerImage();
}

void ImageView::zoomToOriginal()
{
	if (mImage.isNull())
		return;

	// No scale
	mTransform	  = QTransform();
	mInvTransform = QTransform();

	// Shift to center
	centerImage();
}

void ImageView::centerImage()
{
	const QPointF imgCenter	  = QPointF(mImage.width() / 2.0f, mImage.height() / 2.0f);
	const QPointF viewCenter  = mInvTransform.map(QPointF(viewSize().width() / 2.0f, viewSize().height() / 2.0f));
	const QPointF deltaCenter = viewCenter - imgCenter;

	mTransform.translate(deltaCenter.x(), deltaCenter.y());
	mInvTransform = mTransform.inverted();

	repaint();
}

void ImageView::showUpdateRegions(bool b)
{
	mShowUpdateRegions = b;
	repaint();
}

int ImageView::barHeight() const
{
	constexpr int PADDING	   = 2;
	const QFontMetrics metrics = this->fontMetrics();
	return metrics.height() + PADDING * 2;
}

QSize ImageView::viewSize() const
{
	return QSize(width(), height() - barHeight());
}

void ImageView::setView(const std::shared_ptr<ImageBufferView>& view)
{
	mChannelMask   = 0xFF;
	mChannelOffset = 0;
	mView		   = view;
	updateImage();
	updateGeometry();
}

void ImageView::setUpdateRegions(const QVector<QRect>& rects)
{
	mUpdateRegions = rects;
	repaint();
}

void ImageView::setPipeline(const ImagePipeline& pipeline)
{
	mPipeline = pipeline;
	updateImage();
}

void ImageView::exportImage(const QString& path) const
{
	mImage.save(path);
}

QSize ImageView::minimumSizeHint() const
{
	if (!mImage.isNull()) {
		return QSize(qMin(mImage.width(), MIN_S), barHeight() + qMin(mImage.height(), MIN_S));
	} else {
		return QSize(MIN_S, barHeight() + MIN_S);
	}
}

QSize ImageView::sizeHint() const
{
	if (!mImage.isNull()) {
		return QSize(mImage.width(), barHeight() + mImage.height());
	} else {
		return minimumSizeHint();
	}
}

void ImageView::paintEvent(QPaintEvent* event)
{
	const int bar_height = barHeight();

	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.drawPixmap(0, 0, mBackground);

	// Position Field
	painter.setBrush(QBrush(Qt::white));
	painter.drawText(QRect(0, 0, BAR_POS_W / 2 - 1, bar_height - 1),
					 QString::number(mLastPixel.x()),
					 QTextOption(Qt::AlignCenter));
	painter.drawText(QRect(BAR_POS_W / 2 + 1, 0, BAR_POS_W / 2 - 1, bar_height - 1),
					 QString::number(mLastPixel.y()),
					 QTextOption(Qt::AlignCenter));

	// Only populate if there is an active view
	if (mView) {
		const bool additionalField = mView->viewChannelCount() == 1;
		if (additionalField) {
			painter.drawText(QRect(BAR_POS_W + 1, 0, BAR_POS_W / 2 - 1, bar_height - 1),
							 QString::number(mChannelOffset),
							 QTextOption(Qt::AlignCenter));

			painter.drawLine(BAR_POS_W + BAR_POS_W / 2, 0,
							 BAR_POS_W + BAR_POS_W / 2, bar_height - 1);
		}

		// Value Field
		QString valS = valueAt(mLastPixel);
		painter.drawText(QRect(BAR_POS_W + (additionalField ? BAR_POS_W / 2 : 0), 0,
							   width() - BAR_POS_W - 1, bar_height - 1),
						 valS, QTextOption(Qt::AlignCenter));
	}

	// Image
	painter.setClipRect(0, bar_height, width(), height() - bar_height);
	painter.setClipping(true);
	painter.translate(0, bar_height);
	painter.setTransform(mTransform, true);

	painter.drawPixmap(0, 0, mPixmap);

	// Update Rects
	if (mShowUpdateRegions) {
		painter.setPen(QPen(Qt::darkRed));
		painter.setBrush(Qt::NoBrush);
		painter.drawRects(mUpdateRegions);
	}

	event->accept();
}

// Cache background
void ImageView::resizeEvent(QResizeEvent* event)
{
	renderBackground(event->size());
}

void ImageView::renderBackground(const QSize& size)
{
	const int bar_height = barHeight();

	QPainter painter;
	mBackground = QPixmap(size);

	painter.begin(&mBackground);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Bar
	painter.setBrush(QBrush(Qt::darkGray));
	painter.drawRect(0, 0, mBackground.width() - 1, bar_height - 1);

	// Separators
	painter.drawLine(BAR_POS_W / 2, 0,
					 BAR_POS_W / 2, bar_height - 1);
	painter.drawLine(BAR_POS_W, 0,
					 BAR_POS_W, bar_height - 1);

	// Background
	const float grid_s = GRID_S * std::ceil(std::max(logicalDpiX() / (float)width(), logicalDpiY() / (float)height()));
	const int gx	   = (mBackground.width() / (2 * grid_s) + 1);
	const int gy	   = (mBackground.height() - bar_height) / grid_s + 1;

	painter.setPen(Qt::NoPen);
	for (int y = 0; y < gy; ++y) {
		int dx = (y % 2) ? 0 : grid_s;
		int py = bar_height + y * grid_s;

		painter.setBrush(QBrush(Qt::white));
		for (int x = 0; x < gx; ++x)
			painter.drawRect(dx + x * grid_s * 2, py,
							 grid_s, grid_s);

		painter.setBrush(QBrush(Qt::lightGray));
		for (int x = 0; x < gx; ++x)
			painter.drawRect(grid_s - dx + x * grid_s * 2, py,
							 grid_s, grid_s);
	}
	painter.end();
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		mLastPos = event->globalPos();
	}
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		QPointF d = event->globalPos() - mLastPos;

		// No rotation involved, thats why this is fine.
		const float sx = mTransform.m11();
		const float sy = mTransform.m22();

		mTransform.translate(d.x() * PAN_W / sx, d.y() * PAN_W / sy);
		mInvTransform = mTransform.inverted();
		mLastPos	  = event->globalPos();
	}

	QPoint pixel = mapToPixel(event->pos());
	if (isValidPixel(pixel))
		mLastPixel = pixel;

	repaint();
}

void ImageView::wheelEvent(QWheelEvent* event)
{
	const float delta = event->angleDelta().y();
	if (std::abs(delta) < 1e-3f)
		return;

	const QPointF pos = mInvTransform.map(event->posF());
	const float zoom  = delta < 0 ? 1 / ZOOM_W : ZOOM_W;

	mTransform.translate(pos.x(), pos.y());
	mTransform.scale(zoom, zoom);
	mTransform.translate(-pos.x(), -pos.y());
	mInvTransform = mTransform.inverted();

	event->accept();

	repaint();
}

void ImageView::showContextMenu(const QPoint& p)
{
	QMenu contextMenu(tr("Channels"), this);

	if (mView->viewChannelCount() != 1) {
		for (int i = 0; i < mView->channelCount(); ++i) {
			QAction* action = new QAction(
				mView->channelName(i),
				this);
			action->setCheckable(true);
			action->setChecked(mChannelMask & (1 << i));
			action->setData(QVariant::fromValue(i));

			connect(action, &QAction::triggered, [&]() { this->onContextMenuClick(this->sender()); });
			contextMenu.addAction(action);
		}
	} else {
		for (int i = 0; i < mView->channelCount(); ++i) {
			QAction* action = new QAction(
				mView->channelName(i),
				this);
			action->setData(QVariant::fromValue(i));

			connect(action, &QAction::triggered, [&]() { this->onContextMenuClick(this->sender()); });

			contextMenu.addAction(action);
		}
	}

	contextMenu.exec(mapToGlobal(p));
}

void ImageView::onContextMenuClick(QObject* obj)
{
	QAction* act = qobject_cast<QAction*>(obj);
	if (!act)
		return;

	uint id = act->data().toUInt();

	quint8 oldMask = mChannelMask;
	quint32 oldOff = mChannelOffset;
	if (mView->viewChannelCount() != 1) {
		if (act->isChecked())
			mChannelMask |= (1 << id);
		else
			mChannelMask &= ~(1 << id);
	} else {
		mChannelOffset = id;
	}

	if (oldMask != mChannelMask
		|| oldOff != mChannelOffset)
		updateImage();
}

void ImageView::updateImage()
{
	if (mView) {
		mView->fillImage(mImage, mPipeline, mChannelOffset, mChannelMask);
		mPixmap = QPixmap::fromImage(mImage);
	} else {
		mImage	= QImage();
		mPixmap = QPixmap();
	}
	repaint();
}

QPoint ImageView::mapToPixel(const QPoint& pos) const
{
	const QPointF gp = QPointF(pos.x(), pos.y() - (float)barHeight());
	const QPointF lp = mInvTransform.map(gp);
	return QPoint(std::floor(lp.x()), std::floor(lp.y()));
}

bool ImageView::isValidPixel(const QPoint& pixel) const
{
	return pixel.x() >= 0 && pixel.x() < mImage.width()
		   && pixel.y() >= 0 && pixel.y() < mImage.height();
}

QString ImageView::valueAt(const QPoint& pixel) const
{
	QString str;
	for (int i = 0; i < mView->channelCount(); ++i) {
		float v = mView->value(pixel.x(), pixel.y(), i);
		str += QString::number(v, 'g', 2) + " ";
	}

	return str;
}
} // namespace UI
} // namespace PR